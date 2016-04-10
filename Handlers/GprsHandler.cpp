#ifdef ENABLE_GPRS_TESTING
#include "GprsHandler.h"
#include "mbed.h"
#include "config.h"
#include "UsbComms.h"
#include "circbuff.h"
#define TX_GSM P1_27
#define RX_GSM P1_26

// declare led3 to display GPRS handler state
extern DigitalOut myled3;

/*
 * PINPWR to low on Q10 drives 3V3 to Q7, which drives Q5 to ground, which powers VBAT_900, with
 * either VCC_BUCK or VCC_BAT
 */
#define PINPWR                  P1_2
#define PINONOFF                P1_7

#define REQ_SEND_SMS     0b00000001

#define USB_BUFF_SIZE 256

#define SIM900_SERIAL_TIMEOUT 10000

GprsHandler::GprsHandler(MyTimers * _timer, UsbComms *_usb) : AbstractHandler(_timer)
{
    m_serial = new Serial(TX_GSM, RX_GSM);	// create object for UART comms
    mode = gprs_Start;		// initialise state machine

    m_sim900_pwr = new DigitalOut(PINPWR);		// create pin out for
    m_sim900_on  = new DigitalOut(PINONOFF);
    m_reqReg = 0;

    m_usb = _usb;
    m_rxBuff = new CircBuff(USB_BUFF_SIZE);

    m_atReq = atreq_Test;
}

GprsHandler::~GprsHandler()
{
    // TODO Auto-generated destructor stub
    delete m_serial;
    delete m_rxBuff;
    delete m_sim900_pwr;
    delete m_sim900_on;
}

void GprsHandler::run()
{
    switch(mode)
    {
    case gprs_Start:

        mode = gprs_PowerOff;
        break;


        // POWER HANDLERS

    case gprs_PowerOff:
        m_sim900_pwr->write(1);				// turn power supply off
        m_sim900_on->write(1);
        m_timer->SetTimer(MyTimers::tmr_GprsPower, 500);	// wait to settle
        mode = gprs_PowerOffWait;
        break;

    case gprs_PowerOffWait:
        if (!m_timer->GetTimer(MyTimers::tmr_GprsPower))
        {
            mode = gprs_PowerSupplyOn;		// timer has elapsed
        }
        break;

    case gprs_PowerSupplyOn:
        m_sim900_pwr->write(0);		// turn power supply on
        m_sim900_on->write(0);		// from the ref: "drive the PWRKEY to a low level for 1 second then release."


        m_timer->SetTimer(MyTimers::tmr_GprsPower, 1000);	// wait for one second
        mode = gprs_PowerSupplyOnWait;						// go to wait state
        break;

    case gprs_PowerSupplyOnWait:
        if (!m_timer->GetTimer(MyTimers::tmr_GprsPower))
        {
            mode = gprs_PowerSwitchOn;		// timer has elapsed
        }
        break;

    case gprs_PowerSwitchOn:
        m_sim900_on->write(1);		// release power key
        m_timer->SetTimer(MyTimers::tmr_GprsPower, 500);	// wait to settle
        mode = gprs_PowerSwitchOnWait;
        break;

    case gprs_PowerSwitchOnWait:
        if (!m_timer->GetTimer(MyTimers::tmr_GprsPower))
        {
            mode = gprs_CheckATReqs;		// timer has elapsed
        }
        break;


        // REQUEST HANDLERS

    case gprs_CheckATReqs:
        switch (m_atReq) {
        case atreq_Test:
            sprintf((char*)txBuf, "AT\r\n");
            txBufLen = 4;
            mode = gprs_PostTx;
            break;

        case atreq_CheckSMS:
            sprintf((char*)txBuf, "AT+CMGL=\"ALL\"");
            txBufLen = 13;
            mode = gprs_PostTx;
            break;

        default:
            m_atReq = atreq_Test;
        }
        break;


        // TX/RX HANDLERS

    case gprs_PostTx:
        // use putc, other write functions in serial don't really seem to work
        for (int i = 0; i < txBufLen; i++) {
            m_serial->putc(txBuf[i]);
        }

        // make sure buffer is null terminated before printing to USB
        txBuf[txBufLen+1] = 0;
        m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, txBuf);

        // clear the buffer
        txBufLen = 0;

        // set timeout
        m_timer->SetTimer(MyTimers::tmr_GprsRxTx, SIM900_SERIAL_TIMEOUT);

        // wait for a response
        mode = gprs_WaitRx;
        break;

    case gprs_WaitRx:
        if (m_timer->GetTimer(MyTimers::tmr_GprsRxTx))
        {
            // we have not timed out yet
            // we need a generic rx handler here.
            while (m_serial->readable())
            {
                char ch = m_serial->getc();

                // save this to our rx circular buffer
                m_rxBuff->putc(ch);

                mode = gprs_CheckRx;
    			wait(0.1);
            }
            // we have not timed out, and have not got anything back yet
            // keep waiting in this state
        }
        else {
        	m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, (char*)"SIM900 TIMEOUT!");
            mode = gprs_RxTimeout;
        }
        break;

    case gprs_CheckRx:
        if (m_rxBuff->dataAvailable()) {

            // read out
            unsigned char s[50];
            uint16_t len = m_rxBuff->read(s, 50);

            // write to USB
            m_usb->setRequest(UsbComms::usbreq_PrintToTerminalTimestamp, s);

            // process the reply
            switch(m_atReq) {
            case atreq_Test:
                // should have just gotten an ok back
                bool bOk = false;
                for (int i = 0; i < (len - 1); i++) {
                    if ((s[i] == 'O') && (s[i+1] == 'K')) {
                        bOk = true;
                    }
                }

                if (bOk) {
                    myled3 = 1;
                    // so we know that comms are definitely OK.
                    // now check to see what requests need to get fulfilled

                    if (m_reqReg&REQ_SEND_SMS) {
                        // we want to check what sms is
                        m_atReq = atreq_SendSMS;

                        m_reqReg &= ~REQ_SEND_SMS;
                    }
                    else {
                        // no requests, but see if there are any received SMSs
                        m_atReq = atreq_CheckSMS;
                    }
                }
                else {
                    // did not get the reply we were hoping for.
                }
                break;

            default:
                // todo: handle replies for checking/sending SMSs
                m_atReq = atreq_Test;
                break;
            }
        }
        else {

        }
        m_timer->SetTimer(MyTimers::tmr_GprsRxTx, 2000);
        // now that we're done here, go check what needs to get sent to the SIM900 next
        mode = gprs_WaitUntilNextRequest;
        break;
        
    case gprs_WaitUntilNextRequest:
    	if (!m_timer->GetTimer(MyTimers::tmr_GprsRxTx)) {
			mode = gprs_CheckATReqs;
		}
		break;
    case gprs_RxTimeout:
    case gprs_RxError:
    default:
        mode = gprs_Start;
        break;


    }
}

void GprsHandler::setRequest(int request, void *data)
{
    m_lastRequest = (request_t)request;
    switch(request) {
    case gprsreq_SmsSend:
        GprsRequest *req = (GprsRequest*)data;

        // make a copy
        m_lastMessage = *req;     // there are strings, do i have to copy these manually?

        // set the request
        m_reqReg |= REQ_SEND_SMS;
        break;
    }
}
#endif
