#ifndef __TIMERS_H__
#define __TIMERS_H__

#include "mbed.h"

/*!
 * \brief The MyTimers class creates a Ticker and decrements a collection of unsigned longs that act as timers across the program
 *
 *  \sa eTimerType is used to link external callers with a collection of unsigned longs which are decremented
 *  each ms by \sa run().
 *  This acts as a collection of timers.
 *  This collection can be set by \sa SetTimer and retrieved by \sa GetTimer.
 */
class MyTimers
{
public:
    MyTimers();
    ~MyTimers();

    ///< eTimerType identifies each of the timers in program, and is used to set and get a timer
    typedef enum
    {
        tmr_GroveMeasure,       ///< Used in GroveDht22 handler
        tmr_GprsPower,          ///< Used to power the SIM900 on and off
        tmr_GprsRxTx,           ///< Timeout waiting for a response from the SIM900 over the serial line
        tmr_SdWaitError,        ///< Sd card has hit an error, wait before retrying
        tmr_MeasFlash           ///< Flash once every 2 seconds for heartbeat
    } eTimerType;

    //! run is called each time Ticker fires, which is every 1ms, and decrements all timers if necessary
    void run();

    /*!
     * \brief SetTimer sets the value of the timer
     * \param timertype identifies the timer whose value we want to set
     * \param time_ms is the value the timer will be set to, to start counting down from, in ms
     */
    void SetTimer(eTimerType timertype, unsigned long time_ms);

    /*!
     * \brief GetTimer gets the value of a timer
     * \param timertype identifies the timer whose value we want to get
     * \return the value of the timer
     */
    unsigned long GetTimer(eTimerType timertype);

private:
    unsigned long groveMeasureTimer;    ///< current value of timer for \sa tmr_GroveMeasure
    unsigned long gprsPowerTimer;       ///< current value of timer for \sa tmr_GprsPower
    unsigned long gprsRxTxTimer;        ///< current value of timer for \sa tmr_GprsRxTx
    unsigned long sdWaitErrorTimer;     ///< current value of timer for \sa tmr_SdWaitError
    unsigned long measFlashTimer;       ///< current value of timer for \sa tmr_MeasFlash

    Ticker *m_tick;        ///< Used to call \a run a function periodically
};

#endif
