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

#define TWDT_TIMEOUT_S          3
#define TASK_RESET_PERIOD_S     2

/*
 * Macro to check the outputs of TWDT functions and trigger an abort if an
 * incorrect code is returned.
 */
#define CHECK_ERROR_CODE(functionCall, expected) ({                     \
            int retval = functionCall;                                  \
            if(retval != expected){                                     \
                printf("ERROR = %s %d\n", #functionCall, retval);       \
                abort();                                                \
            }                                                           \
})

static TaskHandle_t task_handles[portNUM_PROCESSORS];


//Callback for user tasks created in app_main()
void reset_task0(void *arg)
{
    int i = *(int*) arg;

    
    //Subscribe this task to TWDT, then check if it is subscribed
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS\n", __FUNCTION__, i,  diff);

        //reset the watchdog every 2 seconds
        CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);  //Comment this line to trigger a TWDT timeout
        vTaskDelay(pdMS_TO_TICKS(TASK_RESET_PERIOD_S * 1000));
        
    }
}

//Callback for user tasks created in app_main()
void reset_task1(void *arg)
{
    int i = *(int*) arg;

    
    //Subscribe this task to TWDT, then check if it is subscribed
    CHECK_ERROR_CODE(esp_task_wdt_add(NULL), ESP_OK);
    CHECK_ERROR_CODE(esp_task_wdt_status(NULL), ESP_OK);

    while(1)
    {

        static unsigned long ms;
        unsigned long now=millis();
        unsigned long diff = now -ms;
        ms = now;
        printf("%s core %d time = %d mS\n", __FUNCTION__, i,  diff);

        //reset the watchdog every 2 seconds
        CHECK_ERROR_CODE(esp_task_wdt_reset(), ESP_OK);  //Comment this line to trigger a TWDT timeout
        vTaskDelay(pdMS_TO_TICKS(TASK_RESET_PERIOD_S * 1000));
        
    }
}


void setup_dogs()
{
    int tskParam;
    printf("xxxInitialize TWDT\n");
    //Initialize or reinitialize TWDT
    CHECK_ERROR_CODE(esp_task_wdt_init(TWDT_TIMEOUT_S,false), ESP_OK);

    //Subscribe Idle Tasks to TWDT if they were not subscribed at startup
#ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU0
    printf("go CPU0\n");
    tskParam = 0;
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(0));
    xTaskCreatePinnedToCore(reset_task0, "reset task", 1024 * 2 , &tskParam, 10, &task_handles[0], 0);
#endif
#ifndef CONFIG_TASK_WDT_CHECK_IDLE_TASK_CPU1
    printf("go CPU1\n");
    tskParam = 1;
    esp_task_wdt_add(xTaskGetIdleTaskHandleForCPU(1));
    xTaskCreatePinnedToCore(reset_task1, "reset task", 1024 * 2 , &tskParam, 10, &task_handles[1], 1);
#endif

    //Create user tasks and add them to watchdog
    //for( tskParam = 0; tskParam < portNUM_PROCESSORS; tskParam++)
    //{
    //    CHECK_ERROR_CODE(tskParam, tskParam);
    //    xTaskCreatePinnedToCore(reset_task, "reset task", 1024 * 2 , &tsk, 10, &task_handles[tskParam], tskParam);
    //}
}


void check_dogs()
{
    printf("Delay for 10 seconds\n");
    vTaskDelay(pdMS_TO_TICKS(10000));   //Delay for 10 seconds

    printf("Unsubscribing and deleting tasks\n");
    //Delete and unsubscribe Users Tasks from Task Watchdog, then unsubscribe idle task
    for(int i = 0; i < portNUM_PROCESSORS; i++){
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

