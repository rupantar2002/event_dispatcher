#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <dispatcher.h>

static const char *TAG = __FILE__;

typedef enum
{
    EVENT_SIGNAL_INIT = DISPATCHER_SIGNAL_USER,
    EVENT_SIGNAL_EVENT_ONE,
    EVENT_SIGNAL_EVENT_TWO,
    EVENT_SIGNAL_MAX,
} event_signals_t;

#define QUEUE_ITEM_COUNT (10)
#define QUEUE_ITEM_SIZE (sizeof(dispatcher_eventBase_t))

static uint8_t pgQueueStorage[QUEUE_ITEM_COUNT * QUEUE_ITEM_SIZE] = {0};
static uint8_t pgEventStorage[QUEUE_ITEM_SIZE] = {0};
static dispatcher_base_t gDispatcherStack = {0};
static dispatcher_base_t *pgDispatcher = &gDispatcherStack;

uint8_t StateHandler1(dispatcher_base_t *pDispatcher, dispatcher_eventBase_t const *const pEvent);
uint8_t StateHandler2(dispatcher_base_t *pDispatcher, dispatcher_eventBase_t const *const pEvent);

uint8_t StateHandler1(dispatcher_base_t *pDispatcher, dispatcher_eventBase_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent)) // TODO compress in macro DISPATCHER_EVENT_SIGNAL.
    {
    case DISPATCHER_SIGNAL_ENTRY:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_ENTRY", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_INIT};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_INIT:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_INIT", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_EVENT_ONE};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_ONE:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_ONE", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_EVENT_TWO};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_TWO:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_TWO", __LINE__, __func__);
        status = DISPATCHER_TRANSITION(pDispatcher, StateHandler2);
        break;
    }
    case DISPATCHER_SIGNAL_EXIT:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_EXIT", __LINE__, __func__);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    default:
    {
        ESP_LOGW(TAG, "%d,%s,undefine event signal", __LINE__, __func__);
        status = DISPATCHER_SM_STATUS_IGNORED;
        break;
    }
    }
    return status;
}

uint8_t StateHandler2(dispatcher_base_t *pDispatcher, dispatcher_eventBase_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent)) // TODO compress in macro DISPATCHER_EVENT_SIGNAL.
    {
    case DISPATCHER_SIGNAL_ENTRY:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_ENTRY", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_INIT};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_INIT:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_INIT", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_EVENT_ONE};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_ONE:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_ONE", __LINE__, __func__);
        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_EVENT_TWO};
        dispatcher_Post(pgDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_TWO:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_TWO", __LINE__, __func__);
        status = DISPATCHER_TRANSITION(pDispatcher, StateHandler1);
        break;
    }
    case DISPATCHER_SIGNAL_EXIT:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_EXIT", __LINE__, __func__);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    default:
    {
        ESP_LOGW(TAG, "%d,%s,undefine event signal", __LINE__, __func__);
        status = DISPATCHER_SM_STATUS_IGNORED;
        break;
    }
    }
    return status;
}

void app_main(void)
{
    dispatcher_Init(pgDispatcher,
                    QUEUE_ITEM_SIZE,
                    QUEUE_ITEM_COUNT,
                    pgQueueStorage,
                    pgEventStorage,
                    StateHandler1);

    dispatcher_Start(pgDispatcher, false);

    while (1)
    {
        dispatcher_EventLoop(pgDispatcher);
    }
}
