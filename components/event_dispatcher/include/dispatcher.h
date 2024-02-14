/*! \file   dispatcher.h
    \brief  This file cotains all information related to dispatcher.

    Details.
*/

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_log.h>

/*--------------------------LOGGING----------------------*/

/*! \def    DISPATCHER_LOG_ENABLE (1)
    \brief  Log enable value.
*/
#define DISPATCHER_LOG_ENABLE (1)

/*! \def    DISPATCHER_LOG_DISABLE (0)
    \brief  Log disbale value.
*/
#define DISPATCHER_LOG_DISABLE (0)

/*! \def    DISPATCHER_LOG_STATE
    \brief  Dispatcher logging state,vale DISPATCHER_LOG_ENABLE means enable and
            DISPATCHER_LOG_DISABLE means disabled .
*/
#define DISPATCHER_LOG_STATE DISPATCHER_LOG_ENABLE

#if (DISPATCHER_LOG_STATE == DISPATCHER_LOG_ENABLE)

/*! \def    DISPATCHER_LOG_INFO(tag, format, ...)
    \brief  Logs in info level.
*/
#define DISPATCHER_LOG_INFO(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)

/*! \def    DISPATCHER_LOG_ERROR(tag, format, ...)
    \brief  Logs in error level.
*/
#define DISPATCHER_LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)

/*! \def    DISPATCHER_LOG_DEBUG(tag, format, ...)
    \brief  Logs in debug level.
*/
#define DISPATCHER_LOG_DEBUG(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
#else
#define DISPATCHER_LOG_INFO(tag, format, ...)
#define DISPATCHER_LOG_ERROR(tag, format, ...)
#define DISPATCHER_LOG_DEBUG(tag, format, ...)
#endif

/*-------------------------------------------------------*/

/*-------------------------EVENTS-------------------------*/

/*! \typedef    typedef uint16_t dispatcher_eventSignal_t
    \brief      Represents event signal type .
*/
typedef uint16_t dispatcher_eventSignal_t;

/*! \enum   dispatcher_privateSignals_t
    \brief  Enum represenst all event signals internaly posted by
            dispatcher.
*/
typedef enum
{
    DISPATCHER_SIGNAL_NONE = 0,  /*!< Value of 0, representing NO event signal. */
    DISPATCHER_SIGNAL_ENTRY = 1, /*!< Value of 1, representing ENTRY event signal. */
    DISPATCHER_SIGNAL_EXIT = 2,  /*!< Value of 2, representing EXIT event signal. */
    DISPATCHER_SIGNAL_USER = 3,  /*!< Value of 3, representing USER event signal. */
} dispatcher_privateSignals_t;

/*! \struct  dispatcher_eventBase_t
    \brief   Dispatcher base event structure.
             User defined event structures are used
             to be derived from this structure.
    \example
    \code{c}
             struct myEventStruct{
                dispatcher_eventBase_t base.
                element1;
                element2;
                .
                .
                .
                elementN;
             }
    \endcode
    \warning In user defined event structures the first element
             must be dispatcher_eventBase_t type.
*/
typedef struct
{
    dispatcher_eventSignal_t sig; /*!< Element contains event signal value. */
    /* ... */
} dispatcher_eventBase_t;

/*-------------------------------------------------------*/

/*! \enum   dispatcher_err_t
    \brief  Enum represenst all error state.
*/
typedef enum
{
    DISPATCHER_ERR_CLEAR = 0,       /*!< Value representing no error. */
    DISPATCHER_ERR_NULL_PTR,        /*!< Value representing null pointer error. */
    DISPATCHER_ERR_INVALID_ARGS,    /*!< Value representing invalid arguments error. */
    DISPATCHER_ERR_NOT_INITIALIZED, /*!< Value representing not initialized error. */
    DISPATCHER_ERR_QUEUE_FULL,      /*!< Value representing queue full error. */
    DISPATCHER_ERR_QUEUE_EMPTY,     /*!< Value representing queue empty error. */
    DISPATCHER_ERR_PROCESS_FAIL,    /*!< Value representing process fail error. */
    DISPATCHER_ERR_MAX,             /*!< Value representing num of errors. */
} dispatcher_err_t;

/*! \enum   dispatcher_smStatus_t
    \brief  Enum represenst all state machine status values.
*/
typedef enum
{
    DISPATCHER_SM_STATUS_NONE = 0, /*!< Value 0 representing invalid status. */
    DISPATCHER_SM_STATUS_HANDLED = 1, /*!< Value 1 representing event handled status. */
    DISPATCHER_SM_STATUS_IGNORED = 2, /*!< Value 2 representing event ignored status. */
    DISPATCHER_SM_STATUS_TRANSITION = 3, /*!< Value 3 representing transition status. */
    DISPATCHER_SM_STATUS_MAX = 4, /*!< Value 4 representing num of status. */
} dispatcher_smStatus_t;

/*! \typedef    typedef dispatcher_tagBase dispatcher_base_t
    \brief      A type definition for dispatcher_tagBase.
*/
typedef struct dispatcher_tagBase dispatcher_base_t;

/*! \typedef    typedef uint8_t func(dispatcher_base_t *const pDispatcher,
                                    dispatcher_eventBase_t const *const pEvent) 
                                    dispatcher_stateHandler_t.

    \brief      Dispatcher state handler function type.
    \example
    \code{c}
                uint8_t function(dispatcher_base_t *const pDispatcher,
                                dispatcher_eventBase_t const *const pEvent)
                {
                   dispatcher_smStatus_t status = DISPATCHER_SM_STATUS_IGNORED;
                    .
                    .
                    .
                    retrun status;
                }
    \endcode
*/
typedef uint8_t (*dispatcher_stateHandler_t)(dispatcher_base_t *const pDispatcher,
                                             dispatcher_eventBase_t const *const pEvent);

/*! \struct  dispatcher_tagBase
    \brief   Dispatcher base structure.
             User defined diaptchers structures are used
             to be derived from this structure.
    \example
    \code{c}
             struct myDispatcher{
                struct dispatcher_tagBase base.
                or
                dispatcher_base_t base.
                dispatcher_eventBase_t base.
                element1;
                element2;
                .
                .
                .
                elementN;             }
    \endcode
    \warning In user defined dispatcher structures the first element
             must be  dispatcher_base_t or struct dispatcher_tagBase  type.
*/
struct dispatcher_tagBase
{
    dispatcher_stateHandler_t active; /*!< Element contains active state handler. */
    dispatcher_stateHandler_t next; /*!< Element contains next state handler. */
    uint8_t *eventStorage; /*!< Element contains pointer to a event storage buffer. */
    QueueHandle_t queue; /*!< Element contains event queue handle. */
    StaticQueue_t queueStorage; /*!< Element contains event queue stack. */
};

/*! \def   DISPATCHER_SET_EVENT(pEvent, signal)
    \brief  Set event signal to an event.
    \param pEvent Pointer to event structure.
    \param signal event signal value.
*/
#define DISPATCHER_SET_EVENT(pEvent, signal) (((dispatcher_eventBase_t *)(pEvent))->sig = (signal))

/*! \def   DISPATCHER_GET_SIGNAL(pEvent)
    \brief  Get event signal from an event.
    \param pEvent Pointer to event structure.
*/
#define DISPATCHER_GET_SIGNAL(pEvent) (((dispatcher_eventBase_t *)(pEvent))->sig)

/*! \def   DISPATCHER_TRANSITION(pDispatcher, handler)
    \brief  Transition from active hanlder to next handler. 
    \param pDispatcher Pointer to dispatcher structure.
    \param handler state handler function.
*/
#define DISPATCHER_TRANSITION(pDispatcher, handler) ((((dispatcher_base_t *)(pDispatcher))->next = (handler)), DISPATCHER_SM_STATUS_TRANSITION)

/*! \def   DISPATCHER_INITIALIZE(pDispatcher, eventSize
           , eventCount, pQueueStorage, pEventStorage, pHandler) 
    \brief  Initialize dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param eventSize size of a event structure in bytes.
    \param eventCount max number of events staored in the queue.
    \param pQueueStorage Pointer to queue storage buffer.
    \param pEventStorage Pointer to event storage buffer.
    \param pHandler default state handler function.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
#define DISPATCHER_INITIALIZE(pDispatcher, eventSize, eventCount, pQueueStorage, pEventStorage, pHandler) \
    dispatcher_Init((dispatcher_base_t *)(pDispatcher),                                                   \
                    (uint16_t)(eventSize),                                                                \
                    (uint16_t)(eventCount),                                                               \
                    (uint8_t *)(pQueueStorage),                                                           \
                    (uint8_t *)(pEventStorage),                                                           \
                    (dispatcher_stateHandler_t)(pHandler))

/*! \def   DISPATCHER_START(pDispatcher, userSignal)
    \brief  Start dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param userSignal any value except 0 post DISPATCHER_SIGNAL_USER 
                      after DISPATCHER_SIGNAL_ENTRY.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
#define DISPATCHER_START(pDispatcher, userSignal) \
    dispatcher_Start((dispatcher_base_t *)(pDispatcher), (bool)(userSignal))

/*! \def   DISPATCHER_EVENT_LOOP(pDispatcher)
    \brief  Dispatcher event loop. 
    \param pDispatcher Pointer to dispatcher structure.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
    \warning Should be called in continious loop.
*/
#define DISPATCHER_EVENT_LOOP(pDispatcher) \
    dispatcher_EventLoop((dispatcher_base_t *)(pDispatcher))

/*! \def   DISPATCHER_POST_EVENT(pDispatcher, pEvent)
    \brief  Post event to dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param pEvent Pointer to event structure.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
    \warning Should not be called in ISR or any latency critial opetaion .
*/
#define DISPATCHER_POST_EVENT(pDispatcher, pEvent) \
    dispatcher_Post((dispatcher_base_t *)(pDispatcher), (dispatcher_eventBase_t *)(pEvent))

/*! \def   DISPATCHER_POST_EVENT_FROM_ISR(pDispatcher, pEvent, flags)
    \brief Post event from ISR to dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param pEvent Pointer to event structure.
    \param flags Interrupt flags.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
#define DISPATCHER_POST_EVENT_FROM_ISR(pDispatcher, pEvent, flags) \
    dispatcher_PostFromIsr((dispatcher_base_t *)(pDispatcher),     \
                           (dispatcher_eventBase_t *)(pEvent),     \
                           (int)(flags))


/*! 
    \fn   uint8_t dispatcher_Init(dispatcher_base_t *const pDispatcher,
                        uint16_t itemSize,
                        uint16_t itemCount,
                        uint8_t *queueStorage,
                        uint8_t *eventStorage,
                        dispatcher_stateHandler_t defaultHandler).

    \brief  Initialize dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param itemSize size of a event structure in bytes.
    \param itemCount max number of events queue can store.
    \param queueStorage Pointer to queue storage buffer.
    \param eventStorage Pointer to event storage buffer.
    \param defaultHandler default state handler function.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
uint8_t dispatcher_Init(dispatcher_base_t *const pDispatcher,
                        uint16_t itemSize,
                        uint16_t itemCount,
                        uint8_t *queueStorage,
                        uint8_t *eventStorage,
                        dispatcher_stateHandler_t defaultHandler);

/*! \fn   uint8_t dispatcher_Start(dispatcher_base_t *const pDispatcher,
                                    bool userSignal).
    \brief  Start dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param userSignal any value except 0 post DISPATCHER_SIGNAL_USER 
                      after DISPATCHER_SIGNAL_ENTRY.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
uint8_t dispatcher_Start(dispatcher_base_t *const pDispatcher,
                         bool userSignal);

/*! \fn   uint8_t dispatcher_EventLoop(dispatcher_base_t *const pDispatcher).
    \brief  Dispatcher event loop. 
    \param pDispatcher Pointer to dispatcher structure.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
    \warning Should be called in continious loop.
*/
uint8_t dispatcher_EventLoop(dispatcher_base_t *const pDispatcher);


/*! \fn   dispatcher_Post(dispatcher_base_t *const pDispatcher,
                        dispatcher_eventBase_t const *const pEvent).
    \brief  Post event to dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param pEvent Pointer to event structure.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
    \warning Should not be called in ISR or any latency critial opetaion.
*/
uint8_t dispatcher_Post(dispatcher_base_t *const pDispatcher,
                        dispatcher_eventBase_t const *const pEvent);

/*! \fn   uint8_t dispatcher_PostFromIsr(dispatcher_base_t *const pDispatcher,
                               dispatcher_eventBase_t const *const pEvent,
                               int flags)
    \brief Post event from ISR to dispatcher. 
    \param pDispatcher Pointer to dispatcher structure.
    \param pEvent Pointer to event structure.
    \param flags Interrupt flags.
    \return uint8_t any values except DISPATCHER_ERR_CLEAR represents
            failour.
*/
uint8_t dispatcher_PostFromIsr(dispatcher_base_t *const pDispatcher,
                               dispatcher_eventBase_t const *const pEvent,
                               int flags);

#endif //__DISPATCHER_H__