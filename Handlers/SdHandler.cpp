#include "SdHandler.h"

#include "GroveDht22.h" // for interpreting the result struct
#include "circbuff.h"

#define SD_BUFFER_LEN 256u   // length of circular buffers

// define pins for communicating with SD card
#define PIN_MOSI        P1_22
#define PIN_MISO        P1_21
#define PIN_SCK         P1_20
#define PIN_CS          P1_23

#define DATA_FILE_NAME   "/sd/data.csv"
#define SYSLOG_FILE_NAME "/sd/log.txt"

// declare led that will be used to express state of SD card
extern DigitalOut myled2;

SdHandler::SdHandler(MyTimers * _timer) : AbstractHandler(_timer)
{
    // init file system and files
    m_sdfs = new SDFileSystem(PIN_MOSI, PIN_MISO, PIN_SCK, PIN_CS, "sd");
    m_data = NULL;
    m_syslog = NULL;

    // initialise circular buffers
    m_dataLogBuff = new CircBuff(SD_BUFFER_LEN);
    m_sysLogBuff  = new CircBuff(SD_BUFFER_LEN);
    
    mode = sd_Start;
    m_lastRequest = sdreq_SdNone;
}

SdHandler::~SdHandler()
{
    delete m_dataLogBuff;
    delete m_sysLogBuff;
}

void SdHandler::run()
{
    uint16_t tempInt = 0, tempInt2 = 0;

    unsigned char tempBuff[SD_BUFFER_LEN];

    switch(mode)
    {
    case sd_Start:              /* Set up the state machine */
        // check if card is inserted??
        //m_sdfs->unmount();
        // close both files if necessary
        if (m_data != NULL)
            fclose(m_data);
        if (m_syslog != NULL)
            fclose(m_syslog);

        m_data = NULL;
        m_syslog = NULL;

        //mkdir("/sd", 0777);
        //m_sdfs->mount();
        m_data = fopen(DATA_FILE_NAME, "a");
        m_syslog = fopen(SYSLOG_FILE_NAME, "a");

        if ((m_data != NULL) && (m_syslog != NULL))
        {
            // both opened successfully
            fprintf(m_syslog, "Unit booted OK\n");
            fclose(m_syslog);
            
            // write the header in on startup
            fprintf(m_data, "Timestamp, Temperature (degC), Humidity (pc), Dewpoint\n");            
            fclose(m_data);
            mode = sd_CheckSysLogBuffer;
        }
        break;

    case sd_CheckSysLogBuffer:     /* See if any data should be written to the log file */
        if (m_sysLogBuff->dataAvailable())
        {
            tempInt = m_sysLogBuff->read(tempBuff, SD_BUFFER_LEN);
            tempInt2 = fprintf(m_syslog, (const char*)tempBuff);
            if (tempInt == tempInt2)
            {
                // success
                mode = sd_CheckDataLogBuffer;
            }
            else
            {
                // something went wrong
                m_timer->SetTimer(MyTimers::tmr_SdWaitError, 2000);
                mode = sd_WaitError;
            }
        }
        else {
            mode = sd_CheckDataLogBuffer;
        }
        break;

    case sd_CheckDataLogBuffer:    /* See if any data should be written to the data file */
        if (m_dataLogBuff->dataAvailable())
        {            
            m_data = fopen(DATA_FILE_NAME, "a");

            if (m_data != NULL)
            {
                // opened successfully            
                tempInt = m_dataLogBuff->read(tempBuff, SD_BUFFER_LEN);
                tempInt2 = fprintf(m_data, (const char*)tempBuff);            
                fclose(m_data);
                if (tempInt == tempInt2)
                {
                    // success
                    myled2 = 0;
                    mode = sd_CheckSysLogBuffer;
                }
                else
                {
                    // something went wrong
                    m_timer->SetTimer(MyTimers::tmr_SdWaitError, 2000);
                    mode = sd_WaitError;
                }
            }            
            else
            {
                // something went wrong
                m_timer->SetTimer(MyTimers::tmr_SdWaitError, 2000);
                mode = sd_WaitError;
            }
        }
        else {
            mode = sd_CheckSysLogBuffer;
        }
        break;

    case sd_WaitError:     /* Many fails, much wow, wait for a while */
        if (!m_timer->GetTimer(MyTimers::tmr_SdWaitError))
        {
            // timer has elapsed. go back to start.
            mode = sd_Start;
        }
        break;
    }
}

void SdHandler::setRequest(int request, void *data)
{
    request_t req = (request_t)request;
    m_lastRequest = req;
    switch(req) {
    case sdreq_SdNone:
    
        break;
    case sdreq_LogData:
            
        myled2 = 1;
        // have received the data struct. cast it
        Dht22Result *result = (Dht22Result*)data;
        
        // write things to sd card buffer
        char s[50]; // buffer
        int temp;   // length of buffer
        csvStart(result->resultTime);
        temp = sprintf(s, "%4.2f", result->lastCelcius);
        csvData(s, temp);
        temp = sprintf(s, "%4.2f",    result->lastHumidity);
        csvData(s, temp);
        temp = sprintf(s, "%4.2f",    result->lastDewpoint);
        csvData(s, temp);
        csvEnd();
        break;
    case sdreq_LogSystem:
        char *str = (char*)data;
        logEvent(str);
        break;
    }
}

void SdHandler::csvStart(time_t _time)
{
    unsigned char sTemp[17];
    
    
    // extract time_t to time info struct
    
    struct tm * timeinfo = localtime(&_time);
    
    // print the formatted timestamp at the start of terminalBuffer, with comma
    sprintf((char*)&sTemp[0], "%04d%02d%02d %02d%02d%02d,", (timeinfo->tm_year + 1900),
                                                    (timeinfo->tm_mon + 1),
                                                    timeinfo->tm_mday,
                                                    timeinfo->tm_hour,
                                                    timeinfo->tm_min,
                                                    timeinfo->tm_sec);
                      
    sTemp[16] = 0;
                          
    m_dataLogBuff->add(sTemp);
}

void SdHandler::csvData(const char * s, int len)
{
    unsigned char sTemp[50];
    int idx = 0;
    // add a column to the buffer
    sprintf((char*)&sTemp[idx], s);
    idx += len;
    sTemp[idx++] = ',';
    sTemp[idx] = 0;
    m_dataLogBuff->add(sTemp);
}

void SdHandler::csvEnd()
{
    unsigned char sTemp[2];
    sTemp[0] = '\n';
    sTemp[1] = 0;
    m_dataLogBuff->add(sTemp);
}

void SdHandler::logEvent(const char * s)
{
}
