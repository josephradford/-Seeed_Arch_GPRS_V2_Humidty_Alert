/*!
 * Project:     Humidity SMS Alert
 * Author:      Joseph Radford
 * Date start:  January 2016
 * Date mod:
 * Target:      Arch GPRS V2
 * Peripherals: SD Card
 *              SIM Card
 *              Grove Humidity and Temperature Sensor Pro (DHT22 on a breakout board)
 *
 *
 * Takes intermittent readings from the humidity temperature sensor. These are written to a timestamped CSV file
 * on the SD card and printed over serial if there is a connection to a terminal window (Screen on Linux, Putty on Windows, etc)
 *
 * When conditions have been met, an SMS alert is sent notifying the recipient of the last sensor readings.
 *
 * States can be requested, and configurations changed, by sending requests over USB serial or SMS.
 *
 * Inputs:
 * \a GroveDht22
 *
 * Outputs:
 * \a SdHandler
 *
 *
 * Inputs and outputs:
 * \a GprsHandler
 * \a UsbComms
 *
 * Linker:
 * \a MeasurementHandler
 *
 * 10/04/2016 v0.0.1 - Writes timestamped humidity data to an SD card
 *
 * Issues:
 * Stops communicating over USB after ~10 mins. 
 * Will not work if USB not present.
 * Will not work if SD card is not present
 * GPRS yet to be implemented
 * RTC is set to 01/01/2000 00:00:00 at startup. No way of setting it yet.
 */

#include "config.h"

#include "mbed.h"
#include "math.h"

// Clocks and timers
#include "DS1337.h"
#include "rtc.h"
#include "timers.h"

// Handlers
#include "Handlers/GroveDht22.h"
#include "Handlers/UsbComms.h"
#include "Handlers/SdHandler.h"
#include "Handlers/measurementhandler.h"

#ifdef ENABLE_GPRS_TESTING
#include "Handlers/GprsHandler.h"
#endif

/* Declare hardware classes */
 
DigitalOut myled1(LED2);    ///< startup, and USB handler when writing occurring (right most)
DigitalOut myled2(LED3);    ///< SD handler - on when buffer pending, off when written. If error, will be on as buffer won't clear
DigitalOut myled3(LED4);    ///< GPRS handler information
DigitalOut myled4(LED1);    ///< measurement handler flashes, periodically to inform running (left most)

//DigitalOut myenable(P0_6);

DS1337 * RTC_DS1337;        ///< RTC interface class

I2C i2c(P0_5, P0_4);        ///< I2C comms - this is required by the RTC - do NOT change the name of the instance


/* Declare helpers */
MyTimers *mytimer;         ///< declare timers class - required for other classes to use timers (do not change name)


/* Declare handlers */
GroveDht22          *grove;         ///< grove humidity sensor handler class
UsbComms            *usbcomms;      ///< reading and writing to usb
SdHandler           *sdhandler;     ///< Writing data to a file on SD
MeasurementHandler  *measure;       ///< Handle measurements, and route data to the user and user to configuration

#ifdef ENABLE_GPRS_TESTING
GprsHandler * gprs; ///< Reading and writing to the SIM900
#endif

#ifdef ENABLE_GPRS_TESTING
#define NUM_HANDLERS 5
#else
#define NUM_HANDLERS 4
#endif

#define PROGRAM_TITLE   "Arch GPRS V2 Alert and Request"
#define PROGRAM_INFO    "v0.0.1, released 09/04/2016"

/*!
 * \brief main pulses LED1, creates all classes, pulses LED1, then runs through all handlers forever
 * \return
 */
int main()
{
    /* start up sequence, so we know we reached main */
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(0.2);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(1);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(0.2);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(1);
    
    /* Declare all classes */

    // create MyTimers object, which can be used for waiting in handlers
    mytimer = new MyTimers();

    // RTC interface class
    RTC_DS1337 = new DS1337();
    attach_rtc(&my_rtc_read, &my_rtc_write, &my_rtc_init, &my_rtc_enabled);

    // set the time to the start of the year 2000, by default
    // this will be removed when time can be set, so that time is not reset each time the program starts.
    tm timeinfo;
    timeinfo.tm_hour = 0;   timeinfo.tm_min = 0; timeinfo.tm_sec = 0;
    timeinfo.tm_year = (2001 - 1900); timeinfo.tm_mon = 0; timeinfo.tm_mday = 1;
    set_time(mktime(&timeinfo));
    
    // declare usbcomms
    usbcomms = new UsbComms(mytimer);
    
    // declare sd handler
    sdhandler = new SdHandler(mytimer);

#ifdef ENABLE_GPRS_TESTING
    gprs = new GprsHandler(mytimer, usbcomms);
    measure = new MeasurementHandler(sdhandler, usbcomms, gprs, mytimer);
#else
    measure = new MeasurementHandler(sdhandler, usbcomms, mytimer);
#endif

    // declare grove
    grove = new GroveDht22(measure, mytimer);

    // put the handlers in an array for easy reference
#ifdef ENABLE_GPRS_TESTING
    AbstractHandler* handlers[] = {grove, usbcomms, sdhandler, measure, gprs};
#else
    AbstractHandler* handlers[] = {grove, usbcomms, measure, sdhandler};
#endif

    // send startup message to terminal
    usbcomms->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)PROGRAM_TITLE);
    usbcomms->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)PROGRAM_INFO);
    usbcomms->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"\r\n******************\r\n");

    // flush output
    fflush(stdout); 

    
    /* Pulse LED1 again to signify start up was successful */
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(0.2);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(1);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(0.2);
    myled1 = 1;
    wait(0.2);
    myled1 = 0;
    wait(1);

    
    while(1) 
    {
        // perform run functions for all handlers, one after the other
        for (int i = 0; i < NUM_HANDLERS; i++) {
            handlers[i]->run();
        }
    }   // while

    for (int i = 0; i < NUM_HANDLERS; i++) {
        delete handlers[i];
    }
}   // main



