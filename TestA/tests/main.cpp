#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include "projdefs.h"

/* ASSERT */

void myassert(int x)
{
    if(x == 0)
        throw 0;
}

/* CONCURRENT ACCESS */

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

extern "C"
stattime_t US_systemRunTime(void)
{
    static stattime_t i = 0;
    return ++i;
}
