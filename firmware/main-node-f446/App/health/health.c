#include "health.h"

bool health_ok(uint32_t alive, uint32_t required)
{
    return (alive & required) == required;
}