#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSlist.h"
#include "TheHeader.h"

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
    struct task_t theTask;

    classTask<(taskFunction_t)NULL> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = &theTask;

    OS_taskCreate(task, priority, taskFunc, taskParam);

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
    struct task_t theTask;

    classTask<&taskDelay1Tick> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = &theTask;

    OS_taskCreate(task, priority, taskFunc, taskParam);

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
    struct task_t theTask;

    classTask<&taskDelayMaxDelay> taskClosure;

    taskFunction_t taskFunc = &taskClosure.task;
    taskParameter_t taskParam = (taskParameter_t)&taskClosure;
    priority_t priority = 0;
    struct task_t* task = &theTask;

    OS_taskCreate(task, priority, taskFunc, taskParam);

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

BOOST_AUTO_TEST_CASE(asdf)
{
    struct task_t theTask1;
    struct task_t theTask2;
    struct task_t theTask3;

    struct task_t* task1 = &theTask1;
    const tick_t ticksToWait1 = 1;
    const priority_t priority1 = 1;
    OS_taskCreate(task1, priority1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_taskDelay(ticksToWait1);

    struct task_t* task2 = &theTask2;
    const tick_t ticksToWait2 = 3;
    const priority_t priority2 = 2;
    OS_taskCreate(task2, priority2, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_taskDelay(ticksToWait2);

    struct task_t* task3 = &theTask3;
    const tick_t ticksToWait3 = 2;
    const priority_t priority3 = 3;
    OS_taskCreate(task3, priority3, (taskFunction_t)NULL, NULL);
    setCurrentTask(task3);
    OS_taskDelay(ticksToWait3);

    BOOST_CHECK_EQUAL(task1->NodeDelay.List, OSstate.BlockedTaskList_NotOverflowed);
    BOOST_CHECK_EQUAL(task2->NodeDelay.List, OSstate.BlockedTaskList_NotOverflowed);
    BOOST_CHECK_EQUAL(task3->NodeDelay.List, OSstate.BlockedTaskList_NotOverflowed);

    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed->Head->Task, task1);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed->Head->Next->Task, task3);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed->Tail->Task, task2);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed->Length, 3);
}

BOOST_AUTO_TEST_SUITE_END()
