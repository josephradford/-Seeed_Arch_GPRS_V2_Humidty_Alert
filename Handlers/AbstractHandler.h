#ifndef __ABSTRACT_HANDLER_H__
#define __ABSTRACT_HANDLER_H__

#include "../timers.h"

/*!
 * \brief The AbstractHandler class is inherited by all handlers. It forms the basis of any handler, by having
 * a simple \a run function, called in main.cpp, and a \a setRequest function which is used to set a request
 * specific to the reimplemented class.
 */
class AbstractHandler
{
public:
    AbstractHandler(MyTimers *_timer) : m_timer(_timer) {}
    ~AbstractHandler() {}

    /*!
     * \brief run is inherited by each Handler class. It is called from the main while loop in main.cpp.
     * It runs through the state machine, specific to each Handler class.
     */
    virtual void run() = 0; // run the handler

    /*!
     * \brief setRequest sets a request that the handler will complete when it is able to
     * \param request unique to the reimplemented class (an enum) that will be completed in the state machine
     * \param message an optional array of information relevant to the \a request
     */
    virtual void setRequest(int request, void *data = 0) = 0;

protected:
    MyTimers *m_timer;  ///< Handler classes use timers to pause in the state machine, and continue after delay has finished
};

#endif // __ABSTRACT_HANDLER_H__
