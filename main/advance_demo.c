#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <dispatcher.h>

static const char *TAG = __FILE__;

/**
 * @brief User define event signals.
 *
 */
typedef enum
{
    EVENT_SIGNAL_INIT = DISPATCHER_SIGNAL_USER,
    EVENT_SIGNAL_EVENT_ONE,
    EVENT_SIGNAL_EVENT_TWO,
    EVENT_SIGNAL_MAX,
} event_signals_t;

/**
 * @brief User define event structure.
 *
 */
typedef struct
{
    /**
     * @warning should be the first element in the structure else it
     *          Not going to work.
     */
    dispatcher_eventBase_t base;
    union
    {
        /**
         * @brief Parameters corrosponds EVENT_SIGNAL_EVENT_ONE
         *
         */
        struct
        {
            int param1;
            float param2;
            /*....add more as per application....*/
        } eventOne;

        /**
         * @brief Parameters corrosponds EVENT_SIGNAL_EVENT_TWO
         *
         */
        struct
        {
            int param1;
            float param2;
        } eventTwo;

    } params; // event parameters for specific events

} appEvent_t;

/**
 * @brief User defined dispatcher.
 *
 */
typedef struct
{
    /**
     * @warning should be the first element in the structure else it
     *          Not going to work.
     */
    dispatcher_base_t base;

} appDispatcher_t;

#define QUEUE_ITEM_COUNT (10)
#define QUEUE_ITEM_SIZE (sizeof(appEvent_t))

static uint8_t pgQueueStorage[QUEUE_ITEM_COUNT * QUEUE_ITEM_SIZE] = {0};
static uint8_t pgEventStorage[QUEUE_ITEM_SIZE] = {0};
static appDispatcher_t gDispatcherStack = {0};
static appDispatcher_t *pgDispatcher = &gDispatcherStack;

uint8_t StateHandler1(appDispatcher_t *const pDispatcher, appEvent_t const *const pEvent);
uint8_t StateHandler2(appDispatcher_t *const pDispatcher, appEvent_t const *const pEvent);

uint8_t StateHandler1(appDispatcher_t *const pDispatcher, appEvent_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent))
    {
    case DISPATCHER_SIGNAL_ENTRY:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_ENTRY", __LINE__, __func__);
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_INIT);
        DISPATCHER_POST_EVENT(pDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_INIT:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_INIT", __LINE__, __func__);
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_ONE);
        event.params.eventOne.param1 = 1234;
        event.params.eventOne.param2 = 0.04575;
        DISPATCHER_POST_EVENT(pDispatcher, &event);
        ESP_LOGI(TAG, "posting : eventOne.param1= %d and eventOne.param2= %f",
                 event.params.eventOne.param1,
                 event.params.eventOne.param2);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_ONE:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_ONE { eventOne.param1=%d ,eventOne.param2=%f }", __LINE__, __func__,
                 pEvent->params.eventOne.param1,
                 pEvent->params.eventOne.param2);

        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_TWO);
        DISPATCHER_POST_EVENT(pDispatcher, &event);
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

uint8_t StateHandler2(appDispatcher_t *const pDispatcher, appEvent_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent))
    {
    case DISPATCHER_SIGNAL_ENTRY:
    {
        ESP_LOGI(TAG, "%d,%s,DISPATCHER_SIGNAL_ENTRY", __LINE__, __func__);
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_INIT);
        DISPATCHER_POST_EVENT(pDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_INIT:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_INIT", __LINE__, __func__);
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_ONE);
        DISPATCHER_POST_EVENT(pDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_ONE:
    {
        ESP_LOGI(TAG, "%d,%s,EVENT_SIGNAL_EVENT_ONE", __LINE__, __func__);
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_TWO);
        DISPATCHER_POST_EVENT(pDispatcher, &event);
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

    DISPATCHER_INITIALIZE(pgDispatcher,
                          QUEUE_ITEM_SIZE,
                          QUEUE_ITEM_COUNT,
                          pgQueueStorage,
                          pgEventStorage,
                          StateHandler1);

    DISPATCHER_START(pgDispatcher, false);

    while (1)
    {
        /**
         * @brief Caution for if someting broke avoid watchdog reset.
         *
         */
        if (DIPATCHER_EVENT_LOOP(pgDispatcher) != DISPATCHER_ERR_CLEAR)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        else
        {
            /**
             * @brief Intentionaly slowing the event loop.
             *
             */
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
