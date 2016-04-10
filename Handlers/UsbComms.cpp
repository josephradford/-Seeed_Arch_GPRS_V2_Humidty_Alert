#include "UsbComms.h"
#include "mbed.h"

#include "USBDevice.h"
#include "USBSerial.h"

#include "circbuff.h"

#define USB_CIRC_BUFF 256

extern DigitalOut myled1; // this led is used to notify state of USB comms

UsbComms::UsbComms(MyTimers *_timer) : AbstractHandler(_timer)
{
    mode = usb_Start;

    m_circBuff = new CircBuff(USB_CIRC_BUFF);

    // Declare serial port for communication with PC over USB
    _serial = new USBSerial;
}

UsbComms::~UsbComms()
{
    delete _serial;
    delete m_circBuff;
}

void UsbComms::run()
{
    unsigned char temp;
    switch(mode) {
    case usb_Start:
        mode = usb_CheckInput;
        break;
    case usb_CheckInput:
        if (_serial->readable()) {
            _serial->getc();
            // todo: do something with it! this is where config events are started
            // uncomment to print a message confirming input
            // _serial->writeBlock((unsigned char*)"getc\r\n", 6);
            //mode = usb_WaitForInput;
            mode = usb_CheckOutput;
        } else {
            mode = usb_CheckOutput;
        }
        break;
    case usb_CheckOutput:
        if (m_circBuff->dataAvailable() && _serial->writeable()) {
            // ensure only 64 bytes or less are written at a time
            unsigned char s[TX_USB_MSG_MAX];
            int len = m_circBuff->read(s, TX_USB_MSG_MAX);
            _serial->writeBlock((unsigned char*)s, len);
            myled1 = 1;

        } else {
            myled1 = 0;
        }
        mode = usb_CheckInput;
        break;
    }
}

void UsbComms::setRequest(int request, void *data)
{
    request_t req = (request_t)request;

    switch (req) {
    case usbreq_PrintToTerminal:
        printToTerminal((char*)data);
        break;
    case usbreq_PrintToTerminalTimestamp:
        printToTerminalEx((char*)data);
        break;
    }
}

void UsbComms::printToTerminal(char *s)
{
    // simply add this string to the circular buffer
    m_circBuff->add((unsigned char*)s);
}

// print the message, but prepend it with a standard timestamp
// YYYYMMDD HHMMSS: 
// 01234567890123456
void UsbComms::printToTerminalEx(char *s)
{
    unsigned char tempBuff[TX_USB_BUFF_SIZE];
    // this won't work! needs to be a circular buffer...
    uint16_t sSize = 0, i = 0, j = 0;

    // get the length of the passed array (it should be null terminated, but include max buffer size for caution)
    for (sSize = 0; (sSize < TX_USB_BUFF_SIZE) && (s[sSize] != 0); sSize++);


    time_t _time = time(NULL); // get the seconds since dawn of time

    // extract time_t to time info struct
    struct tm * timeinfo = localtime(&_time);

    // print the formatted timestamp at the start of terminalBuffer
    sprintf((char*)&tempBuff[0], "%04d%02d%02d %02d%02d%02d:", (timeinfo->tm_year + 1900),
            (timeinfo->tm_mon + 1),
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec);


    // copy the string into the remainder of the terminal buffer
    for (i = 16; i < (16 + sSize); i++) {
        tempBuff[i] = s[j++];
    }

    // add a carriage return and new line to the output buffer
    tempBuff[i++] = '\r';
    tempBuff[i++] = '\n';
    tempBuff[i++] = 0;

    // add it to the circular buffer
    m_circBuff->add(tempBuff);
}


