#include "Random.h"

#define MBED_NO_GLOBAL_USING_DIRECTIVE
#include "mbed.h"

#include <cstdlib>

void seedRandom()
{
    srand(HAL_GetTick());
}

int randomRange(int lower, int upper)
{
    return (std::rand() % (1 + upper - lower)) + lower;
}
