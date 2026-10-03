#include "log.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

static osMutexId_t log_mutex;

static const char *level_to_str(log_level_t level)
{
    switch (level)
    {
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARN:
            return "WARN";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        default:
            return "?";
    }
}

void log_write(log_level_t level, const char *msg){
    if(osKernelGetState() == osKernelRunning){
        osMutexAcquire(log_mutex, osWaitForever);
    }
    printf("[%8lu] %-5s: %s\r\n", HAL_GetTick(), level_to_str(level), msg);
    if(osKernelGetState() == osKernelRunning){
        osMutexRelease(log_mutex);
    }
}

void log_init(void){
    log_mutex = osMutexNew(NULL);
    if(log_mutex == NULL){
        log_write(LOG_LEVEL_ERROR, "log mutex create failed");
    }
}
