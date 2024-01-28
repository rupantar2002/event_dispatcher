/*! \file   dispatcher.h
    \brief  This file cotains all information related to dispatcher.

    Details.
*/

#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_log.h>

/*--------------------------LOGGING----------------------*/
#define DISPATCHER_LOG_ENABLE (1)

#define DISPATCHER_LOG_DISABLE (0)

#define DISPATCHER_LOG_STATE DISPATCHER_LOG_ENABLE

#if (DISPATCHER_LOG_STATE == DISPATCHER_LOG_ENABLE)
#define DISPATCHER_LOG_INFO(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_DEBUG(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
#else
#define DISPATCHER_LOG_INFO(tag, format, ...)
#define DISPATCHER_LOG_ERROR(tag, format, ...)
#define DISPATCHER_LOG_DEBUG(tag, format, ...)
#endif

    /*-------------------------------------------------------*/

    /*-------------------------EVENT-------------------------*/

    typedef unsigned int dispatcher_eventSignal_t;

    typedef enum
    {
        DISPATCHER_SIGNAL_NONE = 0,
        DISPATCHER_SIGNAL_ENTRY = 1,
        DISPATCHER_SIGNAL_EXIT = 2,
        DISPATCHER_SIGNAL_USER = 3,
        DISPATCHER_SIGNAL_MAX = 4,
    } dispatcher_private_signals_t;

    typedef struct
    {
        dispatcher_eventSignal_t sig;
        /* ... */

    } dispatcher_eventBase_t;

    /*-------------------------------------------------------*/

    typedef enum
    {
        DISPATCHER_ERR_CLEAR = 0,
        DISPATCHER_ERR_NULL_PTR,
        DISPATCHER_ERR_INVALID_ARGS,
        DISPATCHER_ERR_NOT_INITIALIZED,
        DISPATCHER_ERR_QUEUE_FULL,
        DISPATCHER_ERR_QUEUE_EMPTY,
        DISPATCHER_ERR_PROCESS_FAIL,
        DISPATCHER_ERR_MAX,
    } dispatcher_err_t;

    typedef enum
    {
        DISPATCHER_SM_STATUS_NONE = 0,
        DISPATCHER_SM_STATUS_HANDLED = 1,
        DISPATCHER_SM_STATUS_IGNORED = 2,
        DISPATCHER_SM_STATUS_TRANSITION = 3,
        DISPATCHER_SM_STATUS_MAX = 4,
    } dispatcher_smStatus_t;

    typedef struct dispatcher_tagBase dispatcher_base_t;

    typedef uint8_t (*dispatcher_stateHandler_t)(dispatcher_base_t *const pDispatcher,
                                                 dispatcher_eventBase_t const *const pEvent);

    struct dispatcher_tagBase
    {
        dispatcher_stateHandler_t active;
        dispatcher_stateHandler_t next;
        uint8_t *eventStorage;
        QueueHandle_t queue;
        StaticQueue_t queueStack;
    };

#define DISPATCHER_GET_SIGNAL(event) (((dispatcher_eventBase_t *)(event))->sig)

#define DISPATCHER_TRANSITION(dispatcher, handler) ((((dispatcher_base_t *)(dispatcher))->next = (handler)), DISPATCHER_SM_STATUS_TRANSITION)

    uint8_t dispatcher_Init(dispatcher_base_t *const pDispatcher,
                            uint16_t itemSize,
                            uint16_t itemCount,
                            uint8_t *queueStorage,
                            uint8_t *eventStorage,
                            dispatcher_stateHandler_t defaultHandler);

    uint8_t dispatcher_Start(dispatcher_base_t *const pDispatcher,
                             bool userSignal);

    uint8_t dispatcher_EventLoop(dispatcher_base_t *const pDispatcher);

    uint8_t dispatcher_Post(dispatcher_base_t *const pDispatcher,
                            dispatcher_eventBase_t const *const pEvent);

    uint8_t dispatcher_PostFromIsr(dispatcher_base_t *const pDispatcher,
                                   dispatcher_eventBase_t const *const pEvent,
                                   int flags);

#ifdef __cplusplus
}
#endif

#endif //__DISPATCHER_H__