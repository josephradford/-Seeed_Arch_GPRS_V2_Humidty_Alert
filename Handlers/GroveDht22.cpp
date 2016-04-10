#include "GroveDht22.h"
#include "measurementhandler.h"

#define GROVE_NUM_RETRIES 10        // number of retries for reading sensor

DigitalOut grovePwr(P1_3);          // if anything else is interfaced to uart/adc/i2c connectors, this will have to change, as they share this enable line

GroveDht22::GroveDht22(MeasurementHandler *_measure, MyTimers * _timer) : AbstractHandler(_timer), m_measure(_measure)
{
    // initialise class variables
    mode            = dht_StartTurnOff;
    _lastCelcius    = 0.0f;
    _lastHumidity   = 0.0f;
    _lastDewpoint   = 0.0f;
    _lastError      = ERROR_NONE;
    _retries        = 0;
    _newInfo        = 0;

    m_sensor = new DHT(P1_14,SEN51035P);        // Use the SEN51035P sensor
}

GroveDht22::~GroveDht22()
{
    delete m_sensor;
}

void GroveDht22::run()
{
    switch(mode)
    {
    case dht_StartTurnOff:
        powerOn(false);
        m_timer->SetTimer(MyTimers::tmr_GroveMeasure, 1000);     // leave off for 1 second
        mode = dht_StartTurnOffWait;
        break;

    case dht_StartTurnOffWait:
        if (!m_timer->GetTimer(MyTimers::tmr_GroveMeasure))      // wait until timer has elapsed
            mode = dht_StartTurnOn;
        // else come back here
        break;

    case dht_StartTurnOn:
        powerOn(true);
        m_timer->SetTimer(MyTimers::tmr_GroveMeasure, 1000);     // wait one second for sensor to settle
        mode = dht_StartTurnOnWait;
        break;

    case dht_StartTurnOnWait:
        if (!m_timer->GetTimer(MyTimers::tmr_GroveMeasure))      // wait until timer has elapsed
            mode = dht_TakeMeasurement;
        // else come back here
        break;

    case dht_TakeMeasurement:
        _lastError = (eError)m_sensor->readData();     // take the measurement (todo: see if nonblocking is available)
        if (_lastError == ERROR_NONE)
        {
            _retries = 0;                   // reset retries as measurement was successful
            _lastCelcius = m_sensor->ReadTemperature(CELCIUS);
            _lastHumidity = m_sensor->ReadHumidity();
            _lastDewpoint = m_sensor->CalcdewPoint(_lastCelcius, _lastHumidity);
            m_timer->SetTimer(MyTimers::tmr_GroveMeasure, 3000); // wait three seconds
            
            // add the date time
            time_t _time = time(NULL); // get the seconds since dawn of time
            Dht22Result data = {_time, _lastCelcius, _lastHumidity, _lastDewpoint};
            m_measure->setRequest(MeasurementHandler::measreq_DhtResult, (void*)&data);
            mode = dht_WaitMeasurement;
        }
        else
        {
            _retries++;     // there was an error. See if we have reached critical retries
            if (_retries >= GROVE_NUM_RETRIES)
            {
                _retries = 0;
                mode = dht_StartTurnOff; // restart the sensor
            }
            else
            {
                // haven't reach critical retries yet, so try again, after waiting
                m_timer->SetTimer(MyTimers::tmr_GroveMeasure, 3000); // wait three seconds
                mode = dht_WaitMeasurement;
            }
            int sendData = (int)_lastError; // just to make sure nothing funny happens with the enum
            m_measure->setRequest(MeasurementHandler::measreq_DhtError, (int*)&sendData);
        }
        _newInfo = 1;
        break;

    case dht_WaitMeasurement:
        if (!m_timer->GetTimer(MyTimers::tmr_GroveMeasure))  // if timer elapsed
            mode = dht_TakeMeasurement;
        // else come back here
        break;

    }
}

void GroveDht22::setRequest(int request, void *data)
{
    // no requests (yet)
}

// check if there is new information and reset the flag once the check occurs
unsigned char GroveDht22::newInfo()
{
    unsigned char retval = _newInfo;
    _newInfo = 0;
    return retval;
}

// turn the enable pin for the peripheral plugins on the Arch GPRS v2
// it is active low and acts on Q1
void GroveDht22::powerOn(unsigned char ON)
{
    if (ON)
        grovePwr = 0;   // active low, will turn power on
    else
        grovePwr = 1;   // will turn power off
}
