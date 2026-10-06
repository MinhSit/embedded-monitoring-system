#ifndef HEALTH_H
#define HEALTH_H

#include <stdint.h>

#define HEALTH_TASK_ACQ     (1U << 0)
#define HEALTH_TASK_STORAGE (1U << 1)
#define HEALTH_TASK_RX      (1U << 2)
#define HEALTH_TASK_ALL     (HEALTH_TASK_ACQ | HEALTH_TASK_STORAGE | HEALTH_TASK_RX)

#endif /* HEALTH_H */