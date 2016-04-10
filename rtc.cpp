#include "rtc.h"
#include "DS1337.h"

// declare reference to interface to the hardware real time clock
extern DS1337* RTC_DS1337;

// RTC enabled flag
static int _rtcEnabled;

time_t my_rtc_read()
{
    time_t  retval = 0;     // time since start of epoch
    tm      _time_tm;

    RTC_DS1337->readTime();

    // extract values from RTC to tm struct
    _time_tm.tm_year = ((int)RTC_DS1337->getYears()) - 1900;       // struct tm stores (year - 1900)
    _time_tm.tm_mon  = (int)RTC_DS1337->getMonths() - 1;         // struct tm stores months 0 - 11, DS1337 stores 1-12
    _time_tm.tm_mday = (int)RTC_DS1337->getDays();
    _time_tm.tm_hour = (int)RTC_DS1337->getHours();
    _time_tm.tm_min  = (int)RTC_DS1337->getMinutes();
    _time_tm.tm_sec  = (int)RTC_DS1337->getSeconds();

    // convert to time_t    
    retval = mktime(&_time_tm);
    if (retval == (time_t) -1)
        return (time_t)1;               // error
    else
        return retval;
}


//https://developer.mbed.org/teams/Seeed/code/Seeed_Arch_GPRS_V2_RTC_HelloWorld/file/6db7dfbab0f1/main.cpp
void my_rtc_write(time_t _time)
{
    // extract time_t to time info struct
    tm * timeinfo = localtime(&_time);

    RTC_DS1337->setSeconds   (timeinfo->tm_sec);
    RTC_DS1337->setMinutes   (timeinfo->tm_min);
    RTC_DS1337->setHours     (timeinfo->tm_hour);
    RTC_DS1337->setDays      (timeinfo->tm_mday);           // day of month
    RTC_DS1337->setDayOfWeek (timeinfo->tm_wday);
    RTC_DS1337->setMonths    (timeinfo->tm_mon + 1);        // struct tm stores months 0 - 11, DS1337 stores 1-12
    RTC_DS1337->setYears     (timeinfo->tm_year + 1900);    // struct tm subtracts 1900 from year

    RTC_DS1337->setTime();
}

void my_rtc_init()
{
    RTC_DS1337->start();
    _rtcEnabled = 1;
}

int my_rtc_enabled()
{
    return _rtcEnabled;
}
