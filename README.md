# Event Dispatcher
#### Event Dispatcher is an event driven finite state machine which is flexiable enough to implement in any embedded project and has an inherent ability to add flexibility of event driven state machine in an embedded application.

### Here the implementaion is for esp-idf framework and free-rtos dependent

# Features
- Queue base event buffer.
- Operation synchronization.
- Can be used in ISR functions.
- Easy to implement custom events with custom parameters.
- Multiple state handler support
- Thread safty (i.e rtos queue).
- Error Logging support.
- Error handling support.


# Basic Operation

1. Define user event signals.
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
#define QUEUE_ITEM_SIZE (sizeof(dispatcher_eventBase_t))

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

# Advance Operation
#### Every thing is similar to basic oparation but this case we are using macros insted of functions.

1. Define user event signals.
```c
typedef enum
{
    EVENT_SIGNAL_INIT = DISPATCHER_SIGNAL_USER,
    EVENT_SIGNAL_EVENT_ONE,
    EVENT_SIGNAL_EVENT_TWO,
    EVENT_SIGNAL_MAX,
} event_signals_t;
```

2. Create your custom event structure.
```c
typedef struct
{
    // must be the first elemet in user 
    // defined event structures.
    dispatcher_eventBase_t base;

    union
    {   
        // used for EVENT_SIGNAL_EVENT_ONE.
        struct
        {
            int param1;
            float param2;
            /*....add more as per application....*/
        } eventOne;

        // used for EVENT_SIGNAL_EVENT_TWO.
        struct
        {
            int param1;
            float param2;
        } eventTwo;

    } params;
} appEvent_t;
```

3. Create your custom dispatcher structure.
```c
typedef struct
{
    // must be the first elemet in user 
    // defined event structures.
    dispatcher_base_t base;

    // any thing can be stored
    // as per application demands

} appDispatcher_t;
```


4. Create queue storage buffer.

```c
#define QUEUE_ITEM_COUNT (10)
#define QUEUE_ITEM_SIZE (sizeof(appEvent_t))

// application use stack allocated queue 
// that needs storage buffer for element .
static uint8_t pgQueueStorage[QUEUE_ITEM_COUNT * QUEUE_ITEM_SIZE] = {0};
```
5. Create event storage buffer.

```c
static uint8_t pgEventStorage[QUEUE_ITEM_SIZE] = {0};

```


6. Create dispatcher.

```c
static appDispatcher_t gDispatcherStack = {0};
// this poinetr declaration is not strictly
// necessary but just for avoiding to 
// &gDispatcherStack this every time 
// we use dispatcher related operations.
static appDispatcher_t *pgDispatcher = &gDispatcherStack;
```
7. Define custom state handlers.

```c


uint8_t StateHandler1(appDispatcher_t *const pDispatcher, appEvent_t const *const pEvent)
{
    dispatcher_smStatus_t status = 0;

    switch (DISPATCHER_GET_SIGNAL(pEvent))
    {
    case DISPATCHER_SIGNAL_ENTRY:
    {
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_INIT:
    {
        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_ONE);

        // now we are passing out own 
        // event parameters with an event
        event.params.eventOne.param1 = 1234;
        event.params.eventOne.param2 = 0.04575;
        DISPATCHER_POST_EVENT(pDispatcher, &event);
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_ONE:
    {   

        // we are successfuly receiving event 
        // and its parameters
        printf("param1=%d ,eventOne.param2=%f }",
        pEvent->params.eventOne.param1,
        pEvent->params.eventOne.param2)

        appEvent_t event;
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_TWO);
        
        DISPATCHER_POST_EVENT(pDispatcher, &event);

        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    case EVENT_SIGNAL_EVENT_TWO:
    {
        status = DISPATCHER_TRANSITION(pDispatcher, StateHandler2);
        break;
    }
    case DISPATCHER_SIGNAL_EXIT:
    {
        status = DISPATCHER_SM_STATUS_HANDLED;
        break;
    }
    default:
    {
        status = DISPATCHER_SM_STATUS_IGNORED;
        break;
    }
    }
    return status;
}
```

8. Initialize and start dispatcher.

```c
void main(void)
{
    // Initialize
    DISPATCHER_INITIALIZE(pgDispatcher,
                          QUEUE_ITEM_SIZE,
                          QUEUE_ITEM_COUNT,
                          pgQueueStorage,
                          pgEventStorage,
                          StateHandler1);

    // Start
    DISPATCHER_START(pgDispatcher, false);


    while (1)
    {
        DISPATCHER_EVENT_LOOP(pgDispatcher)    
    }
}

```

9. Post event from function.

```c
    void function(){
        // create event
        appEvent_t event;
        // set event
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_ONE);
        // set parameters
        event.params.eventOne.param1 = 1234;
        event.params.eventOne.param2 = 0.04575;
        // post event
        DISPATCHER_POST_EVENT(pDispatcher, &event);
    }

```

10. Post event from isr.

```c

    void isr(){
        // create event
        appEvent_t event;
        // set event
        DISPATCHER_SET_EVENT(&event, EVENT_SIGNAL_EVENT_ONE);
        // set parameters
        event.params.eventOne.param1 = 1234;
        event.params.eventOne.param2 = 0.04575;
        // post event
        DISPATCHER_POST_EVENT_FROM_ISR(pDispatcher, &event,true);
    }

```