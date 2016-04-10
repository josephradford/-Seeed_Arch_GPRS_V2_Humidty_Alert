#ifndef GPRSHANDLER_H_
#define GPRSHANDLER_H_

#include "config.h"
#ifdef ENABLE_GPRS_TESTING
/*
 *  This section of the program is currently under development. The rest of the functions
 * can run wtihout this component by using the ENABLE_GPRS_TESTING flag.
 */

#include "USBDevice.h"	// need to include this, so that USBSerial has correct typedefs!
#include "USBSerial.h"
#include "AbstractHandler.h"

#define GPRS_BUF_LEN 20

#define GPRS_MESSAGE_MAXLEN 160
#define GPRS_RECIPIENTS_MAXLEN 20

struct GprsRequest
{
    char message[GPRS_MESSAGE_MAXLEN];
    char recipients[GPRS_RECIPIENTS_MAXLEN];
};
class UsbComms;
class CircBuff;

/*!
 * \brief The GprsHandler class saves recipients and looks after incoming and outgoing messages
 *
 * Options: save recipients internally to this class.
 * Or - request sends a struct that includes recipients list and message string
 */
class GprsHandler : public AbstractHandler
{
public:
	GprsHandler(MyTimers * _timer, UsbComms *_usb);
	virtual ~GprsHandler();

	void run();

    void setRequest(int request, void *data = 0);

    enum request_t{
        gprsreq_GprsNone,       ///< No request (for tracking what the last request was, this is initial value for that)
        gprsreq_SmsSend,        ///< got a string to send to recipient(s)
        gprsreq_SetRecipients   ///< got a string holding the number(s) we want to send
    };

private:
    enum mode_t{
        gprs_Start,          	///< Set up the state machine and the hardware

        // Restart sequence
        gprs_PowerOff,          ///< Ensure module is powered down first, so we know it is in its initial state
        gprs_PowerOffWait,      ///< Give module time to power down
        gprs_PowerSupplyOn,		///< Once module has fully powered down, power back up to begin using it
        gprs_PowerSupplyOnWait,	///< Give time to power on properly
        gprs_PowerSwitchOn,
        gprs_PowerSwitchOnWait,

        gprs_CheckATReqs,       ///< Check was AT request is next

        // read AT+CMGR=?
        // write AT+CMGS=?
        gprs_PostTx,            ///< Send the current buffer to SIM900 and setup timeouts
        gprs_WaitRx,            ///< Wait for a response over serial
        gprs_CheckRx,           ///< Check the response. Go back into state machine depending on response
        
        gprs_WaitUntilNextRequest, 

        gprs_RxTimeout,         ///< no response
        gprs_RxError            ///< wrong response


    };
    mode_t mode;            ///< the current state in the state machine
    mode_t returnMode;      ///< the state to go back to when the expected reply returns from the SIM900

    ///
    /// \brief The at_req enum stores what AT command should be sent/was just sent to the SIM900
    ///
    enum at_req {
        atreq_Test,         ///< Ping the SIM900
        atreq_CheckSMS,     ///< Check if an SMS is available
        atreq_SendSMS       ///< Send an SMS, according to m_lastRequest
    };
    at_req m_atReq;

    request_t m_lastRequest;
    GprsRequest m_lastMessage;

    Serial * m_serial; //!< Serial port for comms with SIM900

    DigitalOut * m_sim900_pwr;	//!< pin used to enable the SIM900 power switch
    DigitalOut * m_sim900_on;	//!< pin used to drive the power key

    uint8_t m_reqReg;   ///< request register

    unsigned char txBuf[GPRS_BUF_LEN];
    uint16_t txBufLen;
    unsigned char rxBuf[GPRS_BUF_LEN];
    
    UsbComms *m_usb;
    CircBuff *m_rxBuff;
    
};

#endif

#endif /* GPRSHANDLER_H_ */

