#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSlist.h"

struct SchedulerFixture {
    SchedulerFixture() {
        OS_init();
        OS_start();
    }
    ~SchedulerFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(Scheduler, SchedulerFixture)

BOOST_AUTO_TEST_CASE(lock_unlock_scheduler)
{
    const schedulerLock_t n = 7;

    for(schedulerLock_t i = 0; i < n; ++i)
    {
        OS_schedulerLock();

        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, i + 1);
    }

    for(schedulerLock_t i = 0; i < n; ++i)
    {
        OS_schedulerUnlock();

        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, n - 1 - i);
    }
}

BOOST_AUTO_TEST_CASE(increment_ticks)
{
    const tick_t n = 7;

    OS_schedulerLock();

    for(tick_t i = 0; i < n; ++i)
    {
        OS_tick();

        BOOST_CHECK_EQUAL(OSstate.DelayedTicks, i + 1);
        BOOST_CHECK_EQUAL(OSstate.Tick, 0);
    }

    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(OSstate.DelayedTicks, 0);
    BOOST_CHECK_EQUAL(OSstate.Tick, n);
}

BOOST_AUTO_TEST_CASE(swap_delayed_task_list)
{
    OSstate.Tick = MAX_DELAY;

    BOOST_CHECK_EQUAL((tick_t)(OSstate.Tick + 1), 0);

    struct taskHeadList_t* notOverflowed = OSstate.BlockedTaskList_NotOverflowed;
    struct taskHeadList_t* overflowed = OSstate.BlockedTaskList_Overflowed;

    OS_tick();

    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed, overflowed);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_Overflowed, notOverflowed);

    BOOST_CHECK_EQUAL(OSstate.Tick, 0);
}

BOOST_AUTO_TEST_SUITE_END()
