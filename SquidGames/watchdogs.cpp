/* Task_Watchdog Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "watchdogs.h"

extern "C" unsigned long millis(void);

/*
    Effectively create a low priority task on each core. If the low priority
    task starves, the dog which is attached to it will trigger a WDT reset
*/

#define TWDT_DOG_TIMER_SEC    4
#define TASK_SLEEP_PERIOD     TWDT_DOG_TIMER_SEC-1 // test should never fail      

// Everything ok is TWDT_DOG_TIMER_SEC > TASK_SLEEP_PERIOD
// DOG will trigger if TWDT_DOG_TIMER_SEC < TASK_SLEEP_PERIOD


/*
 * Macro to check the outputs of TWDT functions and trigger an abort if an
 * incorrect code is returned.
 * It appears each core has a task called xTaskGetIdleTaskHandleForCPUx
 * provided by FreeRTOS.
 *
 * This task will somehow accept other tasks and monitor them.
 * 
 *     Activate task called xTaskGetIdleTaskHandleForCPUx (the monitor) 
 *     then when your task runs, have it register to xTaskGetIdleTaskHandleForCPUx
 */


static TaskHandle_t task_handles[portNUM_PROCESSORS];

static bool bDogInit = false;   // double init causes crash... go figure.

//-------------------------------------------------------------------------

// spawned task for core0. 
void dogLoop0(void *arg)
{
    int i = *(int*) arg;

    
    //Subscribe this task to TWDT, then check if it is subscribed

    // put this thread under control of the WDT thread
    ABORT_ON_FAIL(esp_task_wdt_add(NULL), ESP_OK);

    // did it stick?
    ABORT_ON_FAIL(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS\n", __FUNCTION__, i,  diff);

		kickDog();
		
        vTaskDelay(pdMS_TO_TICKS(TASK_SLEEP_PERIOD * 1000));
        
    }
}

//-------------------------------------------------------------------------

// spawned task for core1. 
void dogLoop1(void *arg)
{
    int i = *(int*) arg;

    
    //Subscribe this task to TWDT, then check if it is subscribed

    // put this thread under control of the WDT thread
    ABORT_ON_FAIL(esp_task_wdt_add(NULL), ESP_OK);

    // did it stick?
    ABORT_ON_FAIL(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS\n", __FUNCTION__, i,  diff);

		kickDog();

        vTaskDelay(pdMS_TO_TICKS(TASK_SLEEP_PERIOD * 1000));
        
    }
}

//-------------------------------------------------------------------------

void test_watchDogs(void)
{
    int tskParam;

    //Initialize only once or it will fault

	if (!bDogInit)
	{
		bDogInit = true;
		
	    printf("Initialize TWDT\n");
	    ABORT_ON_FAIL(esp_task_wdt_init(TWDT_DOG_TIMER_SEC,false), ESP_OK);

    /*
        FreeRTOS has an idle thread that runs on each core (initially suspended?)
        Add the idle thread to be under watchdog task control per core

        If the idle thread cannot run due to starvation, the WDT will come into play
        The user thread should kick the dog faster than the WDT timeout value.

        FreeRTOS DOES NOT provide a idle task for all cores, that is decided at 
        compile time of FreeRTOS. 

        ****Adding a thread to a core that has no built-in idle task
        causes mystery crashes.
    */
	}
	
    #if 0 //#ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0
        printf("starting idle task mon for core 0\n");

        // add the built-in idle task on core 0 to the watchdog.
        esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
        
        // at this point the thead handle for the USER thread is not defined
        // but we will start the USER thread. 

        // When the thread starts running, THEN thread handle can be determined
        // and that value can be added to the watch dog thread monitor

        tskParam = 0;
        xTaskCreatePinnedToCore(dogLoop0, "dogLoop0", 1024 * 2 , &tskParam, 10, &task_handles[0], 0);

    #else
        printf("Note: FreeRTOS not compiled for core 0 watchdogs\n");
    #endif

    #ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU1
        printf("starting idle task mon for core 1\n");

        // add the built-in idle task on core 1 to the watchdog.
        esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));

        // at this point the thead handle for the USER thread is not defined
        // but we will start the USER thread. 

        // When the thread starts running, THEN thread handle can be determined
        // and that value can be added to the watch dog thread monitor

        tskParam = 1;
        xTaskCreatePinnedToCore(dogLoop1, "dogLoop1", 1024 * 2 , &tskParam, 10, &task_handles[1], 1);
    #else
        printf("!!! FreeRTOS not compiled for core 1 watchdogs\n");
    #endif

}
	
//----------------------------------------------------------------------

void shutdown_dogs()
{
	#if 0  // reference only. I doubt I have need for shutting down dogs.
    printf("Delay for 10 seconds\n");
    vTaskDelay(pdMS_TO_TICKS(10000));   //Delay for 10 seconds

    printf("Unsubscribing and deleting tasks\n");

    //Delete and unsubscribe Users Tasks from Task Watchdog, then unsubscribe idle task
    for(int i = 0; i < portNUM_PROCESSORS; i++)
    {
        vTaskDelete(task_handles[i]);   //Delete user task first (prevents the resetting of an unsubscribed task)
        ABORT_ON_FAIL(esp_task_wdt_delete(task_handles[i]), ESP_OK);     //Unsubscribe task from TWDT
        ABORT_ON_FAIL(esp_task_wdt_status(task_handles[i]), ESP_ERR_NOT_FOUND);  //Confirm task is unsubscribed

        //unsubscribe idle task
        ABORT_ON_FAIL(esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(i)), ESP_OK);     //Unsubscribe Idle Task from TWDT
        ABORT_ON_FAIL(esp_task_wdt_status(xTaskGetIdleTaskHandleForCPU(i)), ESP_ERR_NOT_FOUND);      //Confirm Idle task has unsubscribed
    }


    //Deinit TWDT after all tasks have unsubscribed
    ABORT_ON_FAIL(esp_task_wdt_deinit(), ESP_OK);
    ABORT_ON_FAIL(esp_task_wdt_status(NULL), ESP_ERR_INVALID_STATE);     //Confirm TWDT has been deinitialized

    printf("Complete\n");
	#endif 
}
//----------------------------------------------------------------------------------

// CORE 0 has WDT DISABLED when the RTOS was built
// so don't bother putting anything on 0 or it will crash.

#define DEFAULT_CORE 1   

TaskHandle_t spawnTaskAndDog(  TaskFunction_t pvTaskCode,
                                const char * const pcName,
                                const uint32_t usStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority)
{
    int tskParam;
    TaskHandle_t retval;

    //Initialize WDT, doing it again will cause a crash

    if (!bDogInit)
    {
        bDogInit = true;
        ABORT_ON_FAIL(esp_task_wdt_init(TWDT_DOG_TIMER_SEC,false), ESP_OK);

        // add the built-in idle task on core 1 to the watchdog.
        esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(DEFAULT_CORE));
    }

	printf("creating %s\n", pcName); 
    xTaskCreatePinnedToCore(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, &retval, DEFAULT_CORE);
    return retval;

}

//----------------------------------------------------------------------------------

void kickDog(void)
{
	//int ret = esp_task_wdt_reset();
/*	
	printf("*** kick ***\n");
	switch (ret)
	{
		case ESP_OK: 
			printf("Success");
			break;
		case ESP_ERR_INVALID_ARG:
			printf("Error, the task is already unsubscribed\n");
			break;
		case ESP_ERR_INVALID_STATE:
			printf("Error, the TWDT has not been initialized yet\n");
			break;
		default:
			printf("WTF %d 0x%X\n", ret, ret);
			break;
	}
*/	
}

