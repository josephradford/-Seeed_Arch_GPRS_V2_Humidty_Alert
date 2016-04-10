#ifndef __RTC_H__
#define __RTC_H__

#include "mbed.h"

//! The functions needed to configure time in the micro, used in \sa attach_rtc, using the on board DS_1337
/*!
  As there is no on-chip RTC, connecting the time functionality to the DS_1337 enables more convenient use of time

  The function which is called to configure is as such:
  void attach_rtc(time_t (*read_rtc)(void), void (*write_rtc)(time_t), void (*init_rtc)(void), int (*isenabled_rtc)(void))

 read_rtc is assigned \sa my_rtc_read

 write_rtc is assigned \sa my_rtc_write

 init_rtc is assigned \sa my_rtc_init

 isenabled_rtc is assigned \sa my_rtc_enabled
 */


/*!
 * \brief my_rtc_read Interfaces to the on board RTC DS1337 and converts read values to time_t
 * \return the value read from the on board RTC converted to system struct time_t
 */
time_t my_rtc_read();


/*!
 * \brief my_rtc_write Interfaces to the on board RTC DS1337 and converts and writes the time given as time_t
 * \param _time is the time in system time_t format which will be written to DS1337
 */
void my_rtc_write(time_t _time);


/*!
 * \brief my_rtc_init Initialises the RTC DS1337
 */
void my_rtc_init();


/*!
 * \brief my_rtc_enabled Checks if RTC DS1337 is enabled
 * \return 1 if enabled, 0 if not enabled (i.e. \sa my_rtc_init never called)
 */
int my_rtc_enabled();


#endif // __RTC_H__
