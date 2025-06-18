#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
extern TaskHandle_t spawnTaskAndDog(  TaskFunction_t pvTaskCode, const char * const pcName, const uint32_t usStackDepth, void * const pvParameters, UBaseType_t uxPriority);
extern void kickDog(void);
extern void test_watchDogs(void);


