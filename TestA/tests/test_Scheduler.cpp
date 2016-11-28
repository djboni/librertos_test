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

template<void (*func)(taskParameter_t param)>
class classTask {
public:

    int NumCalls = 0;

    static void task(taskParameter_t param) {
        classTask* p = (classTask*)param;
        ++p->NumCalls;

        if(func != NULL)
            func(param);
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

BOOST_AUTO_TEST_CASE(create_task)
{
    classTask<(taskFunction_t)NULL> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = NULL;

    task = OS_taskCreate(priority, taskFunc, taskParam);

    /* OS_taskCreate return value. */
    BOOST_CHECK_EQUAL(task, &OSstate.TaskControlBlocks[0]);

    /* OS_taskCreate TCB initialization. */
    BOOST_CHECK_EQUAL(task->State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(task->Function, taskFunc);
    BOOST_CHECK_EQUAL(task->Parameter, taskParam);
    BOOST_CHECK_EQUAL(task->Priority, priority);
    BOOST_CHECK_EQUAL(task->NodeDelay.List, (struct taskHeadList_t*)NULL);
    BOOST_CHECK_EQUAL(task->NodeEvent.List, (struct taskHeadList_t*)NULL);

    /* OS_taskCreate OSstate. */
    BOOST_CHECK_EQUAL(OSstate.Task[priority], task);
}

void taskDelay1Tick(taskParameter_t param)
{
    (void)param;
    OS_taskDelay(1);
}

BOOST_AUTO_TEST_CASE(delay_task_not_overflowed)
{
    classTask<&taskDelay1Tick> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = NULL;

    task = OS_taskCreate(priority, taskFunc, taskParam);

    /* Check scheduler. */
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 0);
    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 1);
    BOOST_CHECK_EQUAL(task->State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(task->NodeDelay.List, OSstate.BlockedTaskList_NotOverflowed);
    BOOST_CHECK_EQUAL(task->NodeEvent.List, (struct taskHeadList_t*)NULL);

    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 1);
    OS_tick();
    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 2);
}

void taskDelayMaxDelay(taskParameter_t param)
{
    (void)param;
    OS_taskDelay(MAX_DELAY);
}

BOOST_AUTO_TEST_CASE(delay_task_overflowed)
{
    classTask<&taskDelayMaxDelay> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = NULL;

    task = OS_taskCreate(priority, taskFunc, taskParam);

    /* Check scheduler. */
    OS_tick();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 0);
    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 1);
    BOOST_CHECK_EQUAL(task->State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(task->NodeDelay.List, OSstate.BlockedTaskList_Overflowed);
    BOOST_CHECK_EQUAL(task->NodeEvent.List, (struct taskHeadList_t*)NULL);

    OS_tick();
    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 1);

    OSstate.Tick = MAX_DELAY;
    BOOST_CHECK_EQUAL((tick_t)(OSstate.Tick + 1), 0);
    OS_tick();
    OS_scheduler();
    BOOST_CHECK_EQUAL(taskClosure.NumCalls, 2);
}

BOOST_AUTO_TEST_SUITE_END()
