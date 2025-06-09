/* Task_Watchdog Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "watchdogs.h"

/*
    Effectively puts a predefined internal FreeRTOS 'idle' task under a dog
*/
#if 0
static TaskHandle_t task_handles[portNUM_PROCESSORS];

//-------------------------------------------------------------------------

// spawned task for core0. 
void dogLoop0(void *arg)
{
    int i = *(int*) arg;

    
    //Subscribe this task to TWDT, then check if it is subscribed

    // put this thread under control of the WDT thread
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);

    // did it stick?
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS of %d mS\n", __FUNCTION__, i,  diff, TWDT_DOG_TIMER_SEC * 1000);

        //reset the watchdog every 2 seconds
        CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);  //Comment this line to trigger a TWDT timeout

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
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);

    // did it stick?
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS of %d mS\n", __FUNCTION__, i,  diff,TWDT_DOG_TIMER_SEC * 1000);

        //reset the watchdog every X seconds
        CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);  //Comment this line to trigger a TWDT timeout

        vTaskDelay(pdMS_TO_TICKS(TASK_SLEEP_PERIOD * 1000));
        
    }
}

//-------------------------------------------------------------------------

void test_watchDogs()
{
    int tskParam;
    //Initialize or reinitialize TWDT

    printf("Initialize TWDT test\n");
    CHECK_ERROR_CODE(esp_task_wdt_init(TWDT_DOG_TIMER_SEC,false), ESP_OK);

    /*
        "Subscribe Idle Tasks to TWDT if they were not subscribed at startup"

        FreeRTOS has an idle thread that runs on each core.
        Add the idle thread to be under watchdog task control

        The idle thread never kicks the dog, but if it starves WDT will come into play
        The user thread should kick the dog faster than the WDT timeout value.

        FreeRTOS might not provide a idle task for all cores, that is decided at 
        compile time of FreeRTOS. 

        ****Adding a thread to a core that has no built-in in idle task
        causes mystery crashes.
    */

    #ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0
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
        printf("!!! FreeRTOS not compiled for core 0 watchdogs\n");
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

    //Create user tasks and add them to watchdog
    //for( tskParam = 0; tskParam < portNUM_PROCESSORS; tskParam++)
    //{
    //    CHECK_ERROR_CODE(tskParam, tskParam);
    //    xTaskCreatePinnedToCore(reset_task, "reset task", 1024 * 2 , &tsk, 10, &task_handles[tskParam], tskParam);
    //}
}





void shutdown_dogs()
{
    printf("Delay for 10 seconds\n");
    vTaskDelay(pdMS_TO_TICKS(10000));   //Delay for 10 seconds

    printf("Unsubscribing and deleting tasks\n");

    //Delete and unsubscribe Users Tasks from Task Watchdog, then unsubscribe idle task
    for(int i = 0; i < portNUM_PROCESSORS; i++)
    {
        vTaskDelete(task_handles[i]);   //Delete user task first (prevents the resetting of an unsubscribed task)
        CHECK_ERROR_CODE(esp_task_wdt_delete(task_handles[i]), ESP_OK);     //Unsubscribe task from TWDT
        CHECK_ERROR_CODE(esp_task_wdt_status(task_handles[i]), ESP_ERR_NOT_FOUND);  //Confirm task is unsubscribed

        //unsubscribe idle task
        CHECK_ERROR_CODE(esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(i)), ESP_OK);     //Unsubscribe Idle Task from TWDT
        CHECK_ERROR_CODE(esp_task_wdt_status(xTaskGetIdleTaskHandleForCPU(i)), ESP_ERR_NOT_FOUND);      //Confirm Idle task has unsubscribed
    }


    //Deinit TWDT after all tasks have unsubscribed
    CHECK_ERROR_CODE(esp_task_wdt_deinit(), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_ERR_INVALID_STATE);     //Confirm TWDT has been deinitialized

    printf("Complete\n");
 }
#endif

//---------------------------------------------------------------------------------------
static uint16_t dog_ctr = 0;

void watchdog_postfix(TaskHandle_t xHandle)
{
    CHECK_ERROR_CODE(esp_task_wdt_delete(xHandle), ESP_OK);     //Unsubscribe task from TWDT
    CHECK_ERROR_CODE(esp_task_wdt_status(xHandle), ESP_ERR_NOT_FOUND);  //Confirm task is unsubscribed

    dog_ctr--;

    vTaskDelete(xHandle);   //Delete user task first (prevents the resetting of an unsubscribed task)
    printf("%s handle %d", __FUNCTION__, xHandle);

    if (dog_ctr == 0)
    {
        printf("%s no more dogs ... shutting down WDT", __FUNCTION__);
        //unsubscribe idle task, core 1 only supported
        CHECK_ERROR_CODE(esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(1)), ESP_OK);     //Unsubscribe Idle Task from TWDT
        CHECK_ERROR_CODE(esp_task_wdt_status(xTaskGetIdleTaskHandleForCPU(1)), ESP_ERR_NOT_FOUND);      //Confirm Idle task has unsubscribed


        //Deinit TWDT after all tasks have unsubscribed
        CHECK_ERROR_CODE(esp_task_wdt_deinit(), ESP_OK);
        CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_ERR_INVALID_STATE);     //Confirm TWDT has been deinitialized
    }
}

void watchdog_kick(void)
{
    //reset the watchdog every X seconds
    CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);
}

//---------------------------------------------------------------------------------------
void watchdog_prefix(void)
{
    //Subscribe this task to TWDT, then check if it is subscribed

    // put this thread under control of the WDT thread
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);

    // did it stick?
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    dog_ctr++;

}

//---------------------------------------------------------------------------------------
TaskHandle_t watchdog_task(void (*pvTaskCode)(void *), 
                    const char *const pcName, 
                    const uint32_t usStackDepth, 
                    void *const pvParameters, 
                    uint16_t uxPriority)
{
    int tskParam;
    static bool oneTime = false;
    //Initialize or reinitialize TWDT
    TaskHandle_t handle;

    if (!oneTime)
    {
        printf("Initialize TWDT test\n");
        CHECK_ERROR_CODE(esp_task_wdt_init(TWDT_DOG_TIMER_SEC,false), ESP_OK);
    }

    /*
        "Subscribe Idle Tasks to TWDT if they were not subscribed at startup"

        FreeRTOS has an idle thread that runs on each core.
        Add the idle thread to be under watchdog task control

        The idle thread never kicks the dog, but if it starves WDT will come into play
        The user thread should kick the dog faster than the WDT timeout value.

        FreeRTOS might not provide a idle task for all cores, that is decided at 
        compile time of FreeRTOS. 

        ****Adding a thread to a core that has no built-in in idle task
        causes mystery crashes.
    */

    #ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU1
        if (!oneTime)
        {
            printf("starting idle task mon for core 1\n");

            // add the built-in idle task on core 1 to the watchdog.
            esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
        }

        // at this point the thead handle for the USER thread is not defined
        // but we will start the USER thread. 

        // When the thread starts running, THEN thread handle can be determined
        // and that value can be added to the watch dog thread monitor

        xTaskCreatePinnedToCore(pvTaskCode, pcName , usStackDepth , pvParameters, uxPriority, &handle, 1 /*core1*/);
    #else
        printf("!!! FreeRTOS not compiled for core 1 watchdogs\n");
    #endif

    oneTime = true;
    return handle;

}


