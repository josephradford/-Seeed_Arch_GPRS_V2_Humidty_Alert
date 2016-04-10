#include "timers.h"

MyTimers::MyTimers()
{
    // initialise timers
    groveMeasureTimer = 0;
    gprsPowerTimer    = 0;
    gprsRxTxTimer     = 0;
    sdWaitErrorTimer  = 0;
    measFlashTimer    = 0;

    m_tick = new Ticker();
    // configure the ticker object to run every 1ms, and to call \sa run when it does so.
    m_tick->attach(this, &MyTimers::run, 0.001);
}

MyTimers::~MyTimers()
{
    delete m_tick;
}

void MyTimers::run()
{
    // decrement each timer in the class
    if (groveMeasureTimer) groveMeasureTimer--;
    if (gprsPowerTimer   ) gprsPowerTimer--;
    if (gprsRxTxTimer    ) gprsRxTxTimer--;
    if (sdWaitErrorTimer ) sdWaitErrorTimer--;
    if (measFlashTimer   ) measFlashTimer--;
}


void MyTimers::SetTimer(eTimerType timertype, unsigned long time_ms)
{
    // see which timer we have to set, and set it
    switch(timertype)
    {
    case tmr_GroveMeasure:
        groveMeasureTimer = time_ms;
        break;
    case tmr_GprsPower:
        gprsPowerTimer = time_ms;
        break;
    case tmr_GprsRxTx:
        gprsRxTxTimer = time_ms;
        break;
    case tmr_SdWaitError:
        sdWaitErrorTimer = time_ms;
        break;
    case tmr_MeasFlash:
        measFlashTimer = time_ms;
        break;
    }
}

unsigned long MyTimers::GetTimer(eTimerType timertype)
{
    // see which timer we have to get, and return it.
    switch(timertype)
    {
    case tmr_GroveMeasure:
        return groveMeasureTimer;
    case tmr_GprsPower:
        return gprsPowerTimer;
    case tmr_GprsRxTx:
        return gprsRxTxTimer;
    case tmr_SdWaitError:
        return sdWaitErrorTimer;
    case tmr_MeasFlash:
        return measFlashTimer;
    }
    return 0;
}

