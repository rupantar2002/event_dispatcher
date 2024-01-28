#include <dispatcher.h>
#include <string.h>

static const char *TAG = __FILE__;

uint8_t dispatcher_Init(dispatcher_base_t *const pDispatcher,
                        uint16_t itemSize,
                        uint16_t itemCount,
                        uint8_t *queueStorage,
                        uint8_t *eventStorage,
                        dispatcher_stateHandler_t defaultHandler)
{
    if (pDispatcher == NULL || queueStorage == NULL || eventStorage == NULL || defaultHandler == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,null pointer argument", __LINE__);
        return DISPATCHER_ERR_NULL_PTR;
    }

    if (itemSize == 0 || itemCount == 0)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,requied non zero arguments", __LINE__);
        return DISPATCHER_ERR_INVALID_ARGS;
    }

    (void)memset(pDispatcher, 0, sizeof(dispatcher_base_t));
    pDispatcher->queue = xQueueCreateStatic(itemCount,
                                            itemSize,
                                            queueStorage,
                                            &pDispatcher->queueStack);
    if (pDispatcher->queue == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,queue initialization failed", __LINE__);
        return DISPATCHER_ERR_NOT_INITIALIZED;
    }
    pDispatcher->eventStorage = eventStorage;
    pDispatcher->active = defaultHandler;
    return DISPATCHER_ERR_CLEAR;
}

uint8_t dispatcher_Start(dispatcher_base_t *const pDispatcher,
                         bool userSignal)
{

    if (pDispatcher == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,null pointer argument", __LINE__);
        return DISPATCHER_ERR_NULL_PTR;
    }

    if (pDispatcher->active == NULL || pDispatcher->queue == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,dispatcher not initialized", __LINE__);
        return DISPATCHER_ERR_NOT_INITIALIZED;
    }

    uint8_t ret = DISPATCHER_ERR_CLEAR;
    dispatcher_eventBase_t event = {.sig = DISPATCHER_SIGNAL_ENTRY};
    pDispatcher->active(pDispatcher, &event);

    if (userSignal)
    {
        event.sig = DISPATCHER_SIGNAL_USER;
        ret = dispatcher_Post(pDispatcher, &event);
    }

    return ret;
}

uint8_t dispatcher_EventLoop(dispatcher_base_t *const pDispatcher)
{
    if (pDispatcher == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,null pointer argument", __LINE__);
        return DISPATCHER_ERR_NULL_PTR;
    }

    if (pDispatcher->active == NULL || pDispatcher->queue == NULL || pDispatcher->eventStorage == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,dispatcher not initialized", __LINE__);
        return DISPATCHER_ERR_NOT_INITIALIZED;
    }

    if (xQueueReceive(pDispatcher->queue, pDispatcher->eventStorage, portMAX_DELAY) != pdTRUE)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,dequeue operation failed", __LINE__);
        return DISPATCHER_ERR_PROCESS_FAIL;
    }

    uint8_t ret = DISPATCHER_ERR_CLEAR;
    dispatcher_smStatus_t status = 0;
    dispatcher_eventBase_t *pEvent = (dispatcher_eventBase_t *)pDispatcher->eventStorage;

    status = pDispatcher->active(pDispatcher, pEvent);
    if (status == DISPATCHER_SM_STATUS_TRANSITION)
    {
        pEvent->sig = DISPATCHER_SIGNAL_EXIT;
        pDispatcher->active(pDispatcher, pEvent);

        if (pDispatcher->next != NULL)
        {
            pDispatcher->active = pDispatcher->next;
            pEvent->sig = DISPATCHER_SIGNAL_ENTRY;
            pDispatcher->active(pDispatcher, pEvent);
        }
        else
        {
            DISPATCHER_LOG_ERROR(TAG, "%d,invalid transition request,handler is NULL", __LINE__);
            ret = DISPATCHER_ERR_PROCESS_FAIL;
        }
    }

    return ret;
}

uint8_t dispatcher_Post(dispatcher_base_t *const pDispatcher,
                        dispatcher_eventBase_t const *const pEvent)
{
    if (pDispatcher == NULL || pEvent == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,null pointer argument", __LINE__);
        return DISPATCHER_ERR_NULL_PTR;
    }

    if (pDispatcher->queue == NULL)
    {
        DISPATCHER_LOG_ERROR(TAG, "%d,dispatcher not initialized", __LINE__);
        return DISPATCHER_ERR_NOT_INITIALIZED;
    }

    uint8_t ret = DISPATCHER_ERR_CLEAR;

    // TODO event posting timeout is 100 ms should be changed or
    // defined as a macro to make it user friendly
    BaseType_t state = xQueueSend(pDispatcher->queue, (void *)pEvent, pdMS_TO_TICKS(100));

    if (state != pdTRUE)
    {
        if (state == errQUEUE_FULL)
        {
            DISPATCHER_LOG_ERROR(TAG, "%d,queue overflow", __LINE__);
            ret = DISPATCHER_ERR_QUEUE_FULL;
        }
        else
        {
            DISPATCHER_LOG_ERROR(TAG, "%d,process failed", __LINE__);
            ret = DISPATCHER_ERR_PROCESS_FAIL;
        }
    }
    return ret;
}

uint8_t dispatcher_PostFromIsr(dispatcher_base_t *const pDispatcher,
                               dispatcher_eventBase_t const *const pEvent,
                               int flags)
{

    if (pDispatcher == NULL || pEvent == NULL)
    {
        return DISPATCHER_ERR_NULL_PTR;
    }

    if (pDispatcher->queue == NULL)
    {
        return DISPATCHER_ERR_NOT_INITIALIZED;
    }
    uint8_t ret = DISPATCHER_ERR_CLEAR;
    BaseType_t state = xQueueSendFromISR(pDispatcher->queue, (void *)pEvent, flags);

    if (state != pdTRUE)
    {
        if (state == errQUEUE_FULL)
            ret = DISPATCHER_ERR_QUEUE_FULL;
        else
            ret = DISPATCHER_ERR_PROCESS_FAIL;
    }
    return ret;
}