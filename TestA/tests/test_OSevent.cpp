#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSevent.h"
#include "TheHeader.h"

struct OSeventFixture {
    OSeventFixture() {
        OS_init();
        OS_start();
    }
    ~OSeventFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(OSevent, OSeventFixture)

BOOST_AUTO_TEST_CASE(initialize_eventR)
{
    struct eventR_t event;

    OS_eventRInit(&event);

    BOOST_CHECK_EQUAL(event.ListRead.Head, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(initialize_eventRw)
{
    struct eventRw_t event;

    OS_eventRwInit(&event);

    BOOST_CHECK_EQUAL(event.ListRead.Head, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 0);

    BOOST_CHECK_EQUAL(event.ListWrite.Head, (struct taskListNode_t*)&event.ListWrite);
    BOOST_CHECK_EQUAL(event.ListWrite.Tail, (struct taskListNode_t*)&event.ListWrite);
    BOOST_CHECK_EQUAL(event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(prepend_task)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task = NULL;
    task = OS_taskCreate(0, (taskFunction_t)NULL, NULL);
    setCurrentTask(task);
    OS_eventPrePendTask(&event.ListRead, task);

    BOOST_CHECK_EQUAL(task->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(prepend_task_after_first_pended_task)
{
    /* Prepended task must be in the head of the list (last task to be unblocked
     by an interrupt). */

    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task1 = NULL;
    task1 = OS_taskCreate(0, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_eventPrePendTask(&event.ListRead, task1);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    struct task_t* task2 = NULL;
    task2 = OS_taskCreate(1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_eventPrePendTask(&event.ListRead, task2);

    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 2);
}

BOOST_AUTO_TEST_CASE(pend_task)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task = NULL;
    const tick_t ticksToWait = MAX_DELAY;
    const priority_t priority = 0;
    task = OS_taskCreate(priority, (taskFunction_t)NULL, NULL);
    setCurrentTask(task);
    OS_eventPrePendTask(&event.ListRead, task);
    OS_eventPendTask(&event.ListRead, task, ticksToWait);

    BOOST_CHECK_EQUAL(task->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(pend_task_after_first_pended_task)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task1 = NULL;
    const tick_t ticksToWait1 = MAX_DELAY;
    const priority_t priority1 = 0;
    task1 = OS_taskCreate(priority1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_eventPrePendTask(&event.ListRead, task1);
    OS_eventPendTask(&event.ListRead, task1, ticksToWait1);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    struct task_t* task2 = NULL;
    const tick_t ticksToWait2 = MAX_DELAY;
    const priority_t priority2 = 1;
    task2 = OS_taskCreate(priority2, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_eventPrePendTask(&event.ListRead, task2);
    OS_eventPendTask(&event.ListRead, task2, ticksToWait2);

    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 2);

    BOOST_CHECK_LT(priority1, priority2);
}

BOOST_AUTO_TEST_CASE(pend_task_between_tow_pended_task)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task1 = NULL;
    const tick_t ticksToWait1 = MAX_DELAY;
    const priority_t priority1 = 0;
    task1 = OS_taskCreate(priority1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_eventPrePendTask(&event.ListRead, task1);
    OS_eventPendTask(&event.ListRead, task1, ticksToWait1);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    struct task_t* task2 = NULL;
    const tick_t ticksToWait2 = MAX_DELAY;
    const priority_t priority2 = 2;
    task2 = OS_taskCreate(priority2, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_eventPrePendTask(&event.ListRead, task2);
    OS_eventPendTask(&event.ListRead, task2, ticksToWait2);

    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 2);

    struct task_t* task3 = NULL;
    const tick_t ticksToWait3 = MAX_DELAY;
    const priority_t priority3 = 1;
    task3 = OS_taskCreate(priority3, (taskFunction_t)NULL, NULL);
    setCurrentTask(task3);
    OS_eventPrePendTask(&event.ListRead, task3);
    OS_eventPendTask(&event.ListRead, task3, ticksToWait3);

    BOOST_CHECK_EQUAL(task3->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Head->Next, &task3->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 3);

    BOOST_CHECK_LT(priority1, priority2);
    BOOST_CHECK_LT(priority3, priority2);
    BOOST_CHECK_LT(priority1, priority3);
}

BOOST_AUTO_TEST_CASE(pend_task_before_first_pended_task)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task1 = NULL;
    const tick_t ticksToWait1 = MAX_DELAY;
    const priority_t priority1 = 1;
    task1 = OS_taskCreate(priority1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_eventPrePendTask(&event.ListRead, task1);
    OS_eventPendTask(&event.ListRead, task1, ticksToWait1);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    struct task_t* task2 = NULL;
    const tick_t ticksToWait2 = MAX_DELAY;
    const priority_t priority2 = 0;
    task2 = OS_taskCreate(priority2, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_eventPrePendTask(&event.ListRead, task2);
    OS_eventPendTask(&event.ListRead, task2, ticksToWait2);

    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 2);

    BOOST_CHECK_GT(priority1, priority2);
}

BOOST_AUTO_TEST_CASE(unblock_task_of_higher_priority)
{
    struct eventR_t event;
    OS_eventRInit(&event);

    struct task_t* task1 = NULL;
    const tick_t ticksToWait1 = MAX_DELAY;
    const priority_t priority1 = 0;
    task1 = OS_taskCreate(priority1, (taskFunction_t)NULL, NULL);
    setCurrentTask(task1);
    OS_eventPrePendTask(&event.ListRead, task1);
    OS_eventPendTask(&event.ListRead, task1, ticksToWait1);

    struct task_t* task2 = NULL;
    const tick_t ticksToWait2 = MAX_DELAY;
    const priority_t priority2 = 1;
    task2 = OS_taskCreate(priority2, (taskFunction_t)NULL, NULL);
    setCurrentTask(task2);
    OS_eventPrePendTask(&event.ListRead, task2);
    OS_eventPendTask(&event.ListRead, task2, ticksToWait2);
    OS_eventUnblockTasks(&event.ListRead);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);
    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &OSstate.PendingReadyTaskList);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    OS_eventUnblockTasks(&event.ListRead);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &OSstate.PendingReadyTaskList);
    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &OSstate.PendingReadyTaskList);

    BOOST_CHECK_EQUAL(event.ListRead.Head, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, (struct taskListNode_t*)&event.ListRead);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 0);

    BOOST_CHECK_LT(priority1, priority2);
}

BOOST_AUTO_TEST_SUITE_END()
