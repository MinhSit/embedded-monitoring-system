#include <stdio.h>
#include "health.h"

static int fails = 0;

static void check(const char *name, bool got, bool want)
{
    if(got != want){
        printf("FAIL %s\n", name);
        fails++;
    }
    else{
        printf("PASS %s\n", name);
    }
}

int main(void)
{
    check("all alive",      health_ok(HEALTH_TASK_ALL, HEALTH_TASK_ALL), true);
    check("rx missing",     health_ok(HEALTH_TASK_ACQ | HEALTH_TASK_STORAGE, HEALTH_TASK_ALL), false);
    check("none alive",     health_ok(0U, HEALTH_TASK_ALL), false);
    check("extra bits ok",  health_ok(0xFFU, HEALTH_TASK_ALL), true);
    check("required zero",  health_ok(0U, 0U), true);
    check("one required",   health_ok(HEALTH_TASK_STORAGE, HEALTH_TASK_STORAGE), true);

    printf(fails ? "SOME FAILED\n" : "ALL PASS\n");
    return fails;
}