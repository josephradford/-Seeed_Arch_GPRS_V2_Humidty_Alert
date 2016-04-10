#ifndef MEASUREMENTHANDLER_H
#define MEASUREMENTHANDLER_H

#include "AbstractHandler.h"
#include "GroveDht22.h"
#include "config.h"
#ifdef ENABLE_GPRS_TESTING
#include "GprsHandler.h"
#endif

class SdHandler;
class UsbComms;


/*!
 * \brief The MeasurementHandler class forms the link between data generation and data output, and stores settings.
 *
 * Receives requests from \a GroveDht22 when a new measurement has been taken or error has occurred. This handler then
 * sends that information on to \a UsbComms for printing that information to terminal and \a SdHandler for printing
 * that information to the CSV data file.
 *
 * This handler also determines if the necessary conditions have been met to send an SMS. This is based on last measurement,
 * the set alert threshold, and time since last alert was sent. An SMS is sent using \a GprsHandler.
 *
 * Other requests from inputs asking for current measurement states (such as the latest measurement) may come from \a UsbComms
 * or \a GprsHandler. The string inspection and matching is handled here, and responses sent to the data outputs.
 *
 *
 * Flashes LED4 constantly to inform that normal operation is occurring.
 */
class MeasurementHandler : public AbstractHandler
{
public:
#ifdef ENABLE_GPRS_TESTING
    MeasurementHandler(SdHandler *_sd, UsbComms *_usb, GprsHandler *_gprs, MyTimers *_timer);
#else
    MeasurementHandler(SdHandler *_sd, UsbComms *_usb, MyTimers *_timer);
#endif

    void run();

    void setRequest(int request, void *data = 0);

    Dht22Result lastResult() const { return m_lastResult; }

    enum request_t{
        measreq_MeasReqNone,        ///< No request (for tracking what the last request was, this is initial value for that)
        measreq_DhtResult,          ///< Dht22 returned with a result
        measreq_DhtError,           ///< Dht22 returned with an error
#ifdef ENABLE_GPRS_TESTING
        measreq_Status,             ///< We got an SMS asking for the status (time, last error, last result)
#endif
    };

private:
    SdHandler *m_sd;            ///< Reference to write to SD card
    UsbComms *m_usb;            ///< Reference to write to USB serial port

#ifdef ENABLE_GPRS_TESTING
    GprsHandler *m_gprs;        ///< Reference to write to GPRS (SMS)
#endif

    Dht22Result m_lastResult;   ///< Copy of the last result that came from Dht22
    int  m_lastError;           ///< Copy of the last error that came from Dht22

#ifdef ENABLE_GPRS_TESTING
    char m_lastSender[GPRS_RECIPIENTS_MAXLEN];         ///< The last sender of an SMS
#endif

    bool m_flashOn;             ///< LED is currently on when true

    enum mode_t{
        meas_Start,             ///< Set up the state machine
        meas_CheckRequest,      ///< Check request register

#ifdef ENABLE_GPRS_TESTING
        meas_PostStateSMS,      ///< Send an SMS of the last result and state
#endif
        meas_PostResult,        ///< Write the last Grove result to SD and USB
        meas_PostError,         ///< Write the last Grove error to USB (and in future, SD syslog)

        meas_FlashTimer,        ///< Flash an LED on and off so user knows device is still running

        meas_WaitError          ///< Lots of fails, wait for a while
    };
    mode_t mode;

    request_t m_lastRequest;    ///< tracks if result or error was the last request

    uint8_t m_requestRegister;  ///< contains the current pending requests as bitwise flags

};

#endif // MEASUREMENTHANDLER_H
