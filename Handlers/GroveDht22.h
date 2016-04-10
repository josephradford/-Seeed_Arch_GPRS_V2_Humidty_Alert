#ifndef __GROVE_DHT_22_H__
#define __GROVE_DHT_22_H__

#include "DHT.h"
#include "mbed.h"
#include "AbstractHandler.h"

class MeasurementHandler;

/*!
 * \brief The Dht22Result struct is the information read from a Dht22 Grove sensor
 */
struct Dht22Result {
    time_t resultTime;      ///< timestamp of when the result was returned from the Dht22
    float  lastCelcius;     ///< Temperature result (degC)
    float  lastHumidity;    ///< Humidity result
    float  lastDewpoint;    ///< Dewpoint result
};

/*!
 * \brief The GroveDht22 class handles the interface to the DHT22 humidity and temperature sensor.
 *
 * The state machine checks for errors and retries and will power cycle the sensor if there are
 * GROVE_NUM_RETRIES number of retries.

 * The state machine also ensures that at least two seconds is left between readings.

 * At any time the parent class can access the last good readings, or the last error.

 * The newInfo flag exists so that the parent can decide to only notify (print to terminal or otherwise) when there
 * is new information available. Calling the newInfo getter will clear the newInfo flag.
 */
class GroveDht22 : public AbstractHandler
{
public:
    GroveDht22(MeasurementHandler *_measure, MyTimers * _timer);
    ~GroveDht22();

    void run();

    void setRequest(int request, void *data = 0);

    // getters
    float  lastCelcius()  { return _lastCelcius; }
    float  lastHumidity() { return _lastHumidity; }
    float  lastDewPoint() { return _lastDewpoint; }
    eError lastError()    { return _lastError; }
    unsigned char newInfo();

private:
    // state machine
    typedef enum {
        dht_StartTurnOff,       ///< Begin by ensuring the sensor is switched off
        dht_StartTurnOffWait,   ///< Allow it to power down completely
        dht_StartTurnOn,        ///< Turn the sensor on
        dht_StartTurnOnWait,    ///< Allow sensor to settle after powering on
        dht_TakeMeasurement,    ///< Take a measurement, check if valid, update measurment vars, set newInfo
        dht_WaitMeasurement     ///< Wait for 2 seconds between measurements
    } mode_t;

    mode_t mode;            ///< The current state in the state machine

    float _lastCelcius;     ///< Last temperature reading from the Dht22 sensor
    float _lastHumidity;    ///< Last humidity reading from the Dht22 sensor
    float _lastDewpoint;    ///< Last dewpoint calculation from last temp, humidity vales
    unsigned char _newInfo; ///< This flag indicates there is new information (an error, or a measurement)
    int _retries;           ///< Number of bad readings from the Dht22 sensor
    eError _lastError;      ///< The last error, or lack thereof

    /*!
     * \brief powerOn powers the Dht22 on or off, by toggling the enable pin
     * \param ON true to power on, false to power off
     */
    void powerOn(unsigned char ON);

    MeasurementHandler *m_measure;  ///< Reference to send measurement results and errors to for handling

    DHT *m_sensor;                  ///< Interface to hardware DHT sensor
};

#endif // __GROVE_DHT_22_H__
