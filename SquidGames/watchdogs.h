/*
    Effectively create a low priority task on each core. If the low priority
    task starves, the dog which is attached to it will trigger a WDT reset
*/

#define TWDT_DOG_TIMER_SEC    2  // 2 seconds to kick the watchdog
#define TASK_SLEEP_PERIOD     4  // test will sleep for 4 ... dog is always kicked

// Everything ok is TWDT_DOG_TIMER_SEC > TASK_SLEEP_PERIOD
// DOG will trigger if TWDT_DOG_TIMER_SEC < TASK_SLEEP_PERIOD


/*
 * Macro to check the outputs of TWDT functions and trigger an abort if an
 * incorrect code is returned.
 *
 * It appears each core has a task called xTaskGetIdleTaskHandleForCPUx
 * provided by FreeRTOS.
 * This task will somehow accept other tasks and monitor them.
 * 
 *     Activate task called xTaskGetIdleTaskHandleForCPUx (the monitor) 
 *     then when your task runs, have it register to xTaskGetIdleTaskHandleForCPUx
 */


#define ABORT_ON_FAIL(functionCall, expected) ({                     \
            int retval = functionCall;                                  \
            if(retval != expected){                                     \
                printf("ERROR = %s %d\n", #functionCall, retval);       \
                abort();                                                \
            }                                                           \
})

#define WARN_ON_FAIL(functionCall, expected) ({                     \
            int retval = functionCall;                                  \
            if(retval != expected){                                     \
                printf("ERROR = %s %d\n", #functionCall, retval);       \
                delay(3000);                                            \
            }                                                           \
})
            

TaskHandle_t watchdog_task(void  (*pvTaskCode)(void *), const char *const pcName, const uint32_t usStackDepth, void *const pvParameters, uint16_t uxPriority);
void watchdog_postfix(TaskHandle_t x);
void watchdog_prefix(void);
void watchdog_kick(void);




