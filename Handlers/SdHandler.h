#ifndef __SD_HANDLER_H__
#define __SD_HANDLER_H__

#include "SDFileSystem.h"
#include "AbstractHandler.h"

class CircBuff;

/*!
 * \brief The SdHandler class writes messages to file and handles SD card status
 * 
 * A data CSV file is written with timestamps and the result from the GroveDht22.
 * A system log, not yet fully implemented, tracks system events for debugging purposes.
 */
class SdHandler : public AbstractHandler
{
public:
    SdHandler(MyTimers * _timer);
    ~SdHandler();

    void run();

    void setRequest(int request, void *data = 0);

    bool sdOk();

    enum request_t {
        sdreq_SdNone,       ///< to init
        sdreq_LogData,      ///< Send struct containing a result and timestamp. This turns it into a line in a csv file
        sdreq_LogSystem     ///< write raw string to system logging file (errors, events, etc)
    };

private:
    SDFileSystem * m_sdfs;
    FILE * m_data;
    FILE * m_syslog;

    enum mode_t{
        sd_Start,                   ///< Set up the state machine
        sd_CheckSysLogBuffer,       ///< See if any data should be written to the log file
        sd_CheckDataLogBuffer,      ///< See if any data should be written to the data file

        sd_WaitError                ///< Error. wait for a while
    };
    mode_t mode;

    request_t m_lastRequest;
    
    // helpers
    void csvStart(time_t time);
    void csvData(const char * s, int len);
    void csvEnd();
    void logEvent(const char * s);
    
    CircBuff *m_dataLogBuff;        ///< Data waiting to be written to the data CSV file
    CircBuff *m_sysLogBuff;         ///< Data waiting to be written to the system log file (not yet implemented)
};

#endif // __SD_HANDLER_H__
