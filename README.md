# Event Dispatcher
##### Event Dispatcher is an event driven finite state machine which is flexiable enough to implement in any embedded project and has an inherent ability to add flexibility of event driven state machine in an embedded application.

Here the implementaion is for esp-idf framework and free-rtos dependent

# Features
- Queue base event buffer.
- Operation synchronization.
- Easy to implement custom events with custom data.
- Multiple state handler support
- Thread safty (i.e rtos queue).

# Basic Operation

1. Define user events.
```c
typedef enum
{
    EVENT_SIGNAL_INIT = DISPATCHER_SIGNAL_USER,
    EVENT_SIGNAL_EVENT_ONE,
    EVENT_SIGNAL_EVENT_TWO,
    EVENT_SIGNAL_MAX,
} event_signals_t;
```

2. Create queue storage buffer.

```c
#define QUEUE_ITEM_COUNT (10)
#define QUEUE_ITEM_SIZE (sizeof(dispatcher_base_t))

static uint8_t pgQueueStorage[QUEUE_ITEM_COUNT * QUEUE_ITEM_SIZE] = {0};
```
3. Create event storage.

```c
static uint8_t pgEventStorage[QUEUE_ITEM_SIZE] = {0};

```


4. Create dispatcher.

```c
static dispatcher_base_t gDispatcherStack = {0};
static dispatcher_base_t *pgDispatcher = gDispatcherStack;

```
5. Define state handlers.

```c

uint8_t StateHandler1(dispatcher_base_t *pDispatcher, dispatcher_eventBase_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent))
    {
    /*
    *   Global Entry event.
    */
    case DISPATCHER_SIGNAL_ENTRY:
    {
        // Do some work ...

        // go to any event
        // here we are explicitily going to EVENT_SIGNAL_INIT.
        // but as EVENT_SIGNAL_INIT is same as DISPATCHER_SIGNAL_USER
        // we can say dispatcher to post this event after entry.

        dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_INIT};
        dispatcher_Post(pgDispatcher, &event);

        // set status as handled to signify the eevent is handled.
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }

    /**
     *  
     * */

    case EVENT_SIGNAL_INIT:
    {
        // Do some work ...

        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    /*  ......handle outher events..... */
    
    /*
    *   Global Exit event.
    */
    case DISPATCHER_SIGNAL_EXIT:
    {
        // Do some work ...

        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }


    // default case of unhandled events.
    default:
    {
        status = DISPATCHER_SM_STATUS_IGNORED;
        break;
    }
    }
    return status;
}



```

4. Initialize and start dispatcher.

```c
void app_main(void)
{
    // initialization process.
    dispatcher_Init(pgDispatcher,
                    QUEUE_ITEM_SIZE,
                    QUEUE_ITEM_COUNT,
                    pgQueueStorage,
                    pgEventStorage,
                    StateHandler1);

    // start dispatcher 
    // this function start state machine
    // ans post the entry event.
    dispatcher_Start(pgDispatcher, false);

    // just after handling entry event it automaticaly
    // post DISPATCHER_SIGNAL_USER event.
    //dispatcher_Start(pgDispatcher, true);


    while (1)
    {
        // as we are in free-rtos base system
        // we need to call this function in any nonblocking
        // while loop of any rtos task.
        dispatcher_EventLoop(pgDispatcher);
    }
}

```

3. Post event from function.

```c
    void function(){
    // create event
    dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_INIT};
    // post event
    dispatcher_Post(pgDispatcher, &event);
    }

```

3. Post event from isr.

```c

    void isr(){
    // create event
    dispatcher_eventBase_t event = {.sig = EVENT_SIGNAL_INIT};
    // post event
    dispatcher_PostFromIsr(pgDispatcher, &event,pdTRUE);
    }

```