#include "projdefs.h"

static void(*func)(void) = (void(*)(void))0;

void librertos_test_set_concurrent_behavior(void(*f)(void))
{
    func = f;
}

void librertos_test_concurrent_access(void)
{
    if(func != (void(*)(void))0)
        func();
}
