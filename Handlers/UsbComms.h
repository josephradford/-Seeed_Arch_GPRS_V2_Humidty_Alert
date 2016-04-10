#ifndef __USB_COMMS_H__
#define __USB_COMMS_H__
#include "AbstractHandler.h"

#define TX_USB_MSG_MAX 64u       // only send 64 bytes at a time
#define TX_USB_BUFF_SIZE 256u    // the tx buffer can hold up to 256 bytes

class USBSerial;
class CircBuff;

/*!
 * \brief The UsbComms class handles input and output for the serial port connected to a PC
 * The main run() function cycles through checking for input and copying to a buffer to be handled,
 * and checking the output buffer to see if there is anything to be sent out.
 *
 * Data can be queued for output by copying it to the circular buffer
 */
class UsbComms : public AbstractHandler
{
public:
    UsbComms(MyTimers *_timer);
    ~UsbComms();

    void run();

    void setRequest(int request, void *data = 0);
    
    enum request_t{
        usbreq_PrintToTerminal,         ///< Print to terminal normally
        usbreq_PrintToTerminalTimestamp ///< Print to terminal, including the timestamp
    };

private:
    USBSerial *_serial;         ///< Interface to the serial port
    CircBuff *m_circBuff;       ///< Data waiting to be printed to the serial port

    // state machine
    enum mode_t{
        usb_Start,          ///< Set up the state machine
        usb_CheckInput,     ///< See if any data is waiting to be read in - currently does not process anything
        usb_CheckOutput,    ///< See if any data should be sent over serial
    };
    mode_t mode;
    
    // helpers
    void printToTerminal(char *s);  // raw
    void printToTerminalEx(char *s); // add timestamp
};


#endif
