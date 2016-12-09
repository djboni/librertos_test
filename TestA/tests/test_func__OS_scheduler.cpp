#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"

struct test_func__OS_scheduler__Fixture {
    struct task_t Task1;
    struct task_t Task2;

    test_func__OS_scheduler__Fixture() {
        OS_init();
        OS_start();
    }
    ~test_func__OS_scheduler__Fixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

static int taskDelayCount = 0;
void taskDelay(void*) {
    ++taskDelayCount;

    /* Delay so scheduler does not reschedule task to run again. */
    OS_taskDelay(MAX_DELAY);
}

static int taskScheduleCount = 0;
void taskSchedule(void*) {
    ++taskScheduleCount;

    OS_scheduler();

    /* Delay so scheduler does not reschedule task to run again. */
    OS_taskDelay(MAX_DELAY);
}

static int taskCompletionDelayCount = 0;
void taskCompletionDelay(void*) {
    static bool flag = true;

    ++taskCompletionDelayCount;

    if(flag)
    {
        flag = false;
    }
    else
    {
        /* Delay so scheduler does not reschedule task to run again. */
        OS_taskDelay(MAX_DELAY);
    }
}

BOOST_FIXTURE_TEST_SUITE(func__OS_scheduler, test_func__OS_scheduler__Fixture)

BOOST_AUTO_TEST_CASE(run_scheduler_with_no_tasks)
{
    OS_scheduler();
}

BOOST_AUTO_TEST_CASE(run_scheduler_with_scheduler_locked)
{
    OS_schedulerLock();
    OS_scheduler();
    OS_schedulerUnlock();
}

BOOST_AUTO_TEST_CASE(run_one_task)
{
    taskDelayCount = 0;
    OS_taskCreate(&Task1, 0, &taskDelay, 0);

    OS_scheduler();
    BOOST_CHECK_EQUAL(taskDelayCount, 1);
}

BOOST_AUTO_TEST_CASE(run_scheduler_with_task_running)
{
    taskScheduleCount = 0;
    OS_taskCreate(&Task1, LIBRERTOS_MAX_PRIORITY-1, &taskSchedule, 0);

    OS_scheduler();
    BOOST_CHECK_EQUAL(taskScheduleCount, 1);
}

BOOST_AUTO_TEST_CASE(run_task_to_completion_without_block)
{
    taskCompletionDelayCount = 0;
    OS_taskCreate(&Task1, LIBRERTOS_MAX_PRIORITY-1, &taskCompletionDelay, 0);

    OS_scheduler();
    BOOST_CHECK_EQUAL(taskCompletionDelayCount, 2);
}

BOOST_AUTO_TEST_CASE(assert_if_invalid_priority)
{
    const priority_t priority = LIBRERTOS_MAX_PRIORITY;

    BOOST_CHECK_THROW(
        OS_taskCreate(&Task1, priority, NULL, 0),
        int);
}

BOOST_AUTO_TEST_CASE(assert_if_task_already_created)
{
    const priority_t priority = 0;

    OS_taskCreate(&Task1, priority, NULL, 0);

    BOOST_CHECK_THROW(
        OS_taskCreate(&Task2, priority, NULL, 0),
        int);
}

BOOST_AUTO_TEST_SUITE_END()
