#include "measurementhandler.h"
#include "mbed.h"
#include "SdHandler.h"
#include "UsbComms.h"

// declare led4 so we can flash it to reflect state of this handler
extern DigitalOut myled4;

// flags for the request register
#define REQ_RESULT 0b00000001
#define REQ_ERROR  0b00000010
#ifdef ENABLE_GPRS_TESTING
#define REQ_SMS    0b00000100
#endif

#ifdef ENABLE_GPRS_TESTING
MeasurementHandler::MeasurementHandler(SdHandler *_sd, UsbComms *_usb, GprsHandler *_gprs, MyTimers *_timer)
    : AbstractHandler(_timer), m_sd(_sd), m_usb(_usb), m_gprs(_gprs)
#else
MeasurementHandler::MeasurementHandler(SdHandler *_sd, UsbComms *_usb, MyTimers *_timer)
    : AbstractHandler(_timer), m_sd(_sd), m_usb(_usb)
#endif
{
    m_lastError         = ERROR_NONE;
    mode                = meas_Start;
    m_lastRequest       = measreq_MeasReqNone;
    m_flashOn           = false;
    m_requestRegister   = 0;

#ifdef ENABLE_GPRS_TESTING
    for (int i = 0; i < GPRS_RECIPIENTS_MAXLEN; i++) {
        m_lastSender[i] = 0;
    }
#endif
}

void MeasurementHandler::run()
{
    switch(mode) {
    case meas_Start:
        // start here, and come back here if an error has been flushed out and waited
        mode = meas_CheckRequest;
        break;

    case meas_CheckRequest:
        if (m_requestRegister) {
            // check what has been requested, starting from most highest priority
            
#ifdef ENABLE_GPRS_TESTING
            if (m_requestRegister&REQ_SMS) {
                // an SMS has been requested
                mode = meas_PostStateSMS;
            }
            else if (m_requestRegister&REQ_RESULT) {
#else
            if (m_requestRegister&REQ_RESULT) {
#endif
                // a result has been sent, we need to post it
                mode = meas_PostResult;
            }
            else if (m_requestRegister&REQ_ERROR) {
                // an error has been sent, we need to post it
                mode = meas_PostError;
            }
            else {
                // something went wrong, a flag was set that isn't defined
                m_requestRegister = 0;
                mode = meas_FlashTimer;
            }
        }
        else {
            // no requests, check if running led needs to be flashed
            mode = meas_FlashTimer;
        }
        break;

#ifdef ENABLE_GPRS_TESTING
    case meas_PostStateSMS:
        if (m_requestRegister&REQ_SMS) {
            char s[64];
            // usb print
            sprintf(s, "Temperature is %4.2f degC\nHumidity is %4.2f pc\nDew point is %4.2f", m_lastResult.lastCelcius, m_lastResult.lastHumidity, m_lastResult.lastDewpoint);

            GprsRequest req;
            strcpy(req.message, s);
            strcpy(req.recipients, m_lastSender);
            m_gprs->setRequest(GprsHandler::gprsreq_SmsSend, &req);

            // clear the request reqister's sms flag
            m_requestRegister &= ~REQ_SMS;
        }
        mode = meas_CheckRequest;
        break;
#endif

    case meas_PostResult:
        if (m_requestRegister&REQ_RESULT) {
            // we have a result, post it

            // TODO: check when the last result came in. if it has not been very long (< 5s? < 1s?) avoid posting, so we don't hammer it

            char s[50];
            // usb print
            sprintf(s, "Temperature is %4.2f degC", m_lastResult.lastCelcius);
            m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, s);
            sprintf(s, "Humidity is %4.2f pc",      m_lastResult.lastHumidity);
            m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, s);
            sprintf(s, "Dew point is %4.2f ",    m_lastResult.lastDewpoint);
            m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, s);
            
            // post to SD card
            m_sd->setRequest(SdHandler::sdreq_LogData, &m_lastResult);

            // clear the request
            m_requestRegister &= ~REQ_RESULT;
        }

        // go back to check if there are more requests
        mode = meas_CheckRequest;
        break;


    case meas_PostError:
        if (m_requestRegister&REQ_ERROR) {
            // there is an error, check the value of it and post the corresponding string to USB
            // TODO: post to SD syslog
            switch (m_lastError)
            {
            case BUS_BUSY:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"BUSY!");
                break;
            case ERROR_NOT_PRESENT:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"NOT PRESENT");
                break;
            case ERROR_ACK_TOO_LONG:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"TOO LONG");
                break;
            case ERROR_SYNC_TIMEOUT:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"SYNC TIMEOUTr\n");
                break;
            case ERROR_DATA_TIMEOUT:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"DATA TIMEOUT");
                break;
            case ERROR_CHECKSUM:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"CHECKSUM");
                break;
            case ERROR_NO_PATIENCE:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"NO PATIENCE!");
                break;
            default:
                m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"UNKNOWN");
                break;
            }

            // we have posted it, clear the flag
            m_requestRegister &= ~REQ_ERROR;
        }
        // check if there are any more requests
        mode = meas_CheckRequest;
        break;
        
    case meas_FlashTimer:
        // flash timer to know that we are still alive.        
        if (!m_timer->GetTimer(MyTimers::tmr_MeasFlash)) {      // wait until timer has elapsed
            if (m_flashOn) {
                // turn off
                myled4 = 0;
                m_timer->SetTimer(MyTimers::tmr_MeasFlash, 2000); // stay off for 2 seconds
                m_flashOn = false;
            }
            else {
                // turn on
                myled4 = 1;
                m_timer->SetTimer(MyTimers::tmr_MeasFlash, 1000); // stay on for 1 second
                m_flashOn = true;
            }
                
        }
        mode = meas_CheckRequest;
        break;

    case meas_WaitError:
        // TODO: timer
        mode = meas_Start;
        break;
    }
}

void MeasurementHandler::setRequest(int request, void *data)
{
    m_lastRequest = (request_t)request;
    switch(request) {
    case measreq_DhtResult:
        // this should contain time as well
        Dht22Result *result = (Dht22Result*)data;

        // copy the structure into our own structure
        m_lastResult = *result;

        // set the request
        m_requestRegister |= REQ_RESULT;

        break;

    case measreq_DhtError:
        int *iResult = (int*)data;
        m_lastError = *iResult;
        m_requestRegister |= REQ_ERROR;

        break;

#ifdef ENABLE_GPRS_TESTING
    case measreq_Status:
        // need to send SMS of last result
        // the sender's number is the sent data
        char *sender = (char*)data;

        int i = 0;
        // copy the sender's number
        for (i = 0; (i < GPRS_RECIPIENTS_MAXLEN) && (sender[i] != 0); i++) {
            m_lastSender[i] = sender[i];
        }

        // set the rest of the destination array to null
        for (i; i < GPRS_RECIPIENTS_MAXLEN; i++) {
            m_lastSender[i] = 0;
        }

        // set the request
        m_requestRegister |= REQ_SMS;
        break;
#endif
    }
}
