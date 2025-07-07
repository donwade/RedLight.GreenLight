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
#if 0
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
#endif

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

signed nest(signed ok)
{

	char me[20];
	if (!ok) return 0;

	sprintf(me, "LEVEL %d", ok);
	Serial.printf("%s "__TIME__"\n", me);

	ok--;
	nest(ok);
	//dumpAboutStack(me, 1000);
	
}

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

    uint32_t freeStack;

	kickDog();

	nest(3);
	
    freeStack = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("Free stack space 1: ");
    Serial.println(freeStack);		
	Tdelay(1000);
	
	
	dogTaskData *setup = (dogTaskData *) inParam;

	unsigned int size = setup->stackSize;

	size = 200; //freeStack/4 ;// - 256;
	//size = freeStack - 256;
	
	//printf("taking %d from stack\n", size);
	char thisStack[size];
	memset(thisStack, 0x33, size);
	strcpy(thisStack, "HELLO");
	thisStack[size-1] = 'x';
	
	//void *thisStack = alloca(size);

	
    freeStack = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("Free stack space 2: ");
    Serial.println(freeStack);		
	
	dumpAboutStack("top level", 1024); 
		
	//patternMemory(thisStack, size);
	//dumpAbout(thisStack, size * 2);

	kickDog();

#ifdef PROFILE_DOG
	unsigned long *ms = new(unsigned long);
#endif 

	// never ending call loop.
    while(1)
    {
    
#ifdef PROFILE_DOG
        unsigned long now=millis();
        unsigned long diff = now - *ms;
        *ms = now;
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

    if (!bDogInit)
    {
        bDogInit = true;
        ABORT_ON_FAIL(esp_task_wdt_init(TWDT_DOG_TIMER_SEC,false), ESP_OK);

        // add the built-in idle task on core 1 to the watchdog.
        ABORT_ON_FAIL(esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(DEFAULT_CORE)), ESP_OK);
    }

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
	ABORT_ON_FAIL(esp_task_wdt_reset(), ESP_OK);
}
//----------------------------------------------------
#define WIDTH 8

void dumpAbout(void *address, uint32_t aboutSize) 
{

	printf("\n%s %p len=%d\n", __FUNCTION__, address, aboutSize);
	
	uint8_t *base = (uint8_t *)address;
	int delta = aboutSize / (WIDTH *2) ;

	for (int i = -delta; i < delta; i++)
	{
		kickDog();
		if (!i) printf("\n");
		
		uint8_t *down = base + i * WIDTH;
		printf ("0x%p [%d]:\t", down, i * WIDTH);
		
		int across;
		for ( across = 0; across < WIDTH; across++)
		{
			uint8_t c = down[ across ];
			printf("%02X ", c);
		}
		printf("   ");
		
		for ( across = 0; across < WIDTH; across++)
		{
			uint8_t c = down[ across ];
			printf("%c", c < 0x20 ? '.' : c > 0x7F ? '.' : c);
		}

		if (!i) printf("\n");
		printf("\n");

		// must allow idle to have a go as this is i/o intensive
		// a yeild will NOT work as idle is the lowest priority
		// and this function always be on the READY queue.
		
		delay(1); // go idle task.
	}

	printf("\n");
}

uint8_t *patternMemory(void * where, uint32_t size)
{
	size = (size / 8) * 8;
 	
	uint8_t *foo = (uint8_t*) where;
	int x;
	int r;
	for (int i = 0; i < size; i+=8)
	{
		x = i;
		if (i)
		{
			foo[0] = 'D';
			foo[1] = 'E';
			foo[2] = 'A';
			foo[3] = 'D';
			
			r = x/1000; foo[4] = 0x30+ r; x -= r * 1000;
			r = x/100;  foo[5] = 0x30+ r; x -= r * 100;
			r = x/10;   foo[6] = 0x30+ r; x -= r * 10;
			            foo[7] = 0x30+ x; 
			foo += 8;
		}
		else
		{
			// first entry will be "CODEFOOD"
			*foo++ = 'C';
			*foo++ = 'O';
			*foo++ = 'D';
			*foo++ = 'E';
			*foo++ = 'F';
			*foo++ = 'O';
			*foo++ = 'O';
			*foo++ = 'D';
		}
	 }
	
 	return foo;  // stop optimizaton
}

inline void dumpAboutStack(char *msg, uint32_t aboutSize) 
{
	printf("uuuuuuuuuuu\n");
	char mark[30];
	strncpy(mark, msg, sizeof(mark));
	dumpAbout(mark, 1024);
}


/*
void * testDump(void uint32_t stackSize) 
{
	char foo[] = "FIRSTCALL";
	char *bar = (char*) alloca(stackSize);

	// +33 don't stomp next caller.
	patternMemory(bar + 32, stackSize/2);
	dumpAbout(bar + 32, stackSize/2);
	return foo;  // stop optimization out.
}

*/
