#include "log.h"
#include <stdio.h>
#include "stm32f4xx_hal.h"

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
    printf("[%8lu] %-5s: %s\r\n", HAL_GetTick(), level_to_str(level), msg);
}
