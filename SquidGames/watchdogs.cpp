/* Task_Watchdog Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "watchdogs.h"

extern "C" unsigned long millis(void);

/*
    Effectively create a low priority task on each core. If the low priority
    task starves, the dog which is attached to it will trigger a WDT reset
*/

#define TWDT_DOG_TIMER_SEC    3
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

void setup_watchdogs(void)
{

	// Configure the Task Watchdog Timer
	esp_task_wdt_config_t twdt_config = {
		.timeout_ms = 5000, // Set timeout to 5 seconds
		.idle_core_mask = (1 << configNUM_CORES) - 1, // Monitor all cores' idle tasks
		.trigger_panic = true, // Trigger a panic (and reboot) on timeout
	};

    // Initialize the TWDT with the specified configuration
    ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));

}
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

    //while(1)
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

    //while (1)
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

// CORE 0 has WDT DISABLED when the RTOS was built
// so don't bother putting anything on 0 or it will crash.

#define DEFAULT_CORE 1   

typedef struct dogTaskData
{ 
	TaskFunction_t pvTaskCode;
	void *pvParameters;
	uint32_t stackSize;
	char name[20];
};

//-------------------------------------------
// allow long delays past watchdog
void Tdelay(unsigned int ms)
{
	//printf("\n%s ms=%d\n", __FUNCTION__, ms);	
	while (ms  > TWDT_DOG_TIMER_SEC * 1000)
	{
		kickDog();
		delay(TWDT_DOG_TIMER_SEC * 1000 - 1);
		ms -= TWDT_DOG_TIMER_SEC * 1000;
		//printf("%s in loop ms = %d (wd=%d)\n", __FUNCTION__, ms, TWDT_DOG_TIMER_SEC * 1000);
	}

	//printf("%s exited loop ... ms left = %d\n", __FUNCTION__,  ms);

	kickDog();
	if (ms > 0) delay(ms);
}

//-------------------------------------------

//#define PROFILE_DOG

#include "freertos/FreeRTOS.h"

void onEntryDog(void * const inParam)
{
	{
	    //	Add this task to TWDT
	    //  Only being inside thread can do this.
	    //	Then see if it the RTOS kept it.

	    // put this thread under control of the WDT thread
	    ABORT_ON_FAIL(esp_task_wdt_add(NULL), ESP_OK);

	    // did it stick?
	    ABORT_ON_FAIL(esp_task_wdt_status(NULL), ESP_OK);

 	} // this has to be done before any dog kicks !

	dogTaskData *setup = (dogTaskData *) inParam;

	delay(10);
    uint32_t freeStack;

	kickDog();

#ifdef PROFILE_DOG
	unsigned long ms=millis();
    unsigned long now;
    unsigned long diff;
    printf("*********** START TIME = %d\n", now);
#endif 

	// never ending call loop.
    while(1)
    {
    
#ifdef PROFILE_DOG
        now=millis();
        diff = now - ms;

        ms = now;
        printf("***** %s dog time = %d mS\n", setup->name, diff);
#endif

		setup->pvTaskCode(setup->pvParameters);
		
		delay(1);

    }
}
//-------------------------------------------------------------
TaskHandle_t spawnTaskAndDogV2(  TaskFunction_t pvTaskCode,
                                const char * const pcName,
                                const uint32_t usStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority)
{
    int tskParam;
    TaskHandle_t retval;

    //Initialize WDT, doing it again will cause a crash

	printf("%s creating %s\n", __FUNCTION__, pcName);

	dogTaskData *passIn = (dogTaskData*) malloc(sizeof(dogTaskData));

	passIn->pvParameters = pvParameters;
	passIn->pvTaskCode = pvTaskCode;
	passIn->stackSize = usStackDepth;
	strncpy(passIn->name, pcName, sizeof(passIn->name));
	passIn->name[sizeof(passIn->name)-1] = '\0';

    xTaskCreatePinnedToCore(onEntryDog, pcName, usStackDepth, passIn, uxPriority, &retval, DEFAULT_CORE);
    return retval;

}

//-------------------------------------------------------------

TaskHandle_t spawnTaskAndDog(  TaskFunction_t pvTaskCode,
                                const char * const pcName,
                                const uint32_t usStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority)
{
    int tskParam;
    TaskHandle_t retval;

    //Initialize WDT, doing it again will cause a crash

	printf("creating  %s\n", pcName); 
    xTaskCreatePinnedToCore(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, &retval, DEFAULT_CORE);
    return retval;
}

//----------------------------------------------------------------------------------

void kickDog(void)
{
	ABORT_ON_FAIL(esp_task_wdt_reset(), ESP_OK);
}
//----------------------------------------------------

