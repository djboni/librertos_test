#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSlist.h"
#include "TheHeader.h"

struct test_func__OS_schedulerUnlock__Fixture {
    struct task_t Task1;
    struct task_t Task2;

    test_func__OS_schedulerUnlock__Fixture() {
        OS_init();
        OS_start();
    }
    ~test_func__OS_schedulerUnlock__Fixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(func__OS_schedulerUnlock, test_func__OS_schedulerUnlock__Fixture)

BOOST_AUTO_TEST_CASE(lock_unlock_scheduler)
{
    OS_schedulerLock();
    BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 1);
    OS_schedulerUnlock();
    BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
}

BOOST_AUTO_TEST_CASE(lock2_unlock2_scheduler)
{
    OS_schedulerLock();
    OS_schedulerLock();
    BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 2);
    OS_schedulerUnlock();
    BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 1);
    OS_schedulerUnlock();
}

BOOST_AUTO_TEST_CASE(process_delayed_ticks)
{
    OS_schedulerLock();
    OS_tick();
    BOOST_CHECK_EQUAL(OSstate.Tick, 0);
    BOOST_CHECK_EQUAL(OSstate.DelayedTicks, 1);
    OS_schedulerUnlock();
    BOOST_CHECK_EQUAL(OSstate.Tick, 1);
    BOOST_CHECK_EQUAL(OSstate.DelayedTicks, 0);
}

BOOST_AUTO_TEST_CASE(invert_blocked_tasks_list)
{
    OS_schedulerLock();
    OSstate.Tick = MAX_DELAY;
    OS_tick();

    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_Overflowed, &OSstate.BlockedTaskList2);

    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_NotOverflowed, &OSstate.BlockedTaskList2);
    BOOST_CHECK_EQUAL(OSstate.BlockedTaskList_Overflowed, &OSstate.BlockedTaskList1);
}

BOOST_AUTO_TEST_CASE(task_not_unblocked_by_tick)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    OS_schedulerLock();
    OS_taskDelay(2);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    OS_tick();
    OS_schedulerUnlock();
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
}

BOOST_AUTO_TEST_CASE(task_unblocked_by_tick)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    OS_schedulerLock();
    OS_taskDelay(1);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    OS_tick();
    OS_schedulerUnlock();
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(task_unblocked_by_tick_with_no_task_running)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);

    OS_taskDelay(1);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);

    setCurrentTask(NULL);

    OS_schedulerLock();
    OS_tick();
    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);

}

BOOST_AUTO_TEST_CASE(task_unblocked_by_tick_with_lower_priority_task_running)
{
    OS_taskCreate(&Task1, 1, 0, 0);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);

    OS_taskCreate(&Task2, 0, 0, 0);
    BOOST_CHECK_EQUAL(Task2.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);

    setCurrentTask(&Task1);
    OS_taskDelay(1);
    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);

    setCurrentTask(&Task2);

    OS_schedulerLock();
    OS_tick();
    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(task_unblocked_from_event_by_tick)
{
    struct taskHeadList_t list;

    OS_listHeadInit(&list);

    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_listInsertAfter(&list, list.Head, &Task1.NodeEvent);
    Task1.State = TASKSTATE_SUSPENDED;

    OS_schedulerLock();
    OS_taskDelay(1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &list);

    OS_tick();
    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(task_unblock_suspended_pending_ready_task_with_no_task_running)
{
    struct taskHeadList_t* list = &OSstate.PendingReadyTaskList;

    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_listInsertAfter(list, list->Head, &Task1.NodeEvent);
    Task1.State = TASKSTATE_SUSPENDED;

    setCurrentTask(NULL);

    OS_schedulerLock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_SUSPENDED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &OSstate.PendingReadyTaskList);

    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(task_unblock_suspended_pending_ready_task_with_lower_priority_task_running)
{
    struct taskHeadList_t* list = &OSstate.PendingReadyTaskList;

    OS_taskCreate(&Task1, 1, 0, 0);
    OS_taskCreate(&Task2, 0, 0, 0);

    setCurrentTask(&Task1);
    OS_listInsertAfter(list, list->Head, &Task1.NodeEvent);
    Task1.State = TASKSTATE_SUSPENDED;

    setCurrentTask(&Task2);

    OS_schedulerLock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_SUSPENDED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &OSstate.PendingReadyTaskList);

    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(task_unblock_blocked_pending_ready_task)
{
    struct taskHeadList_t* list = &OSstate.PendingReadyTaskList;

    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_taskDelay(1);
    OS_listInsertAfter(list, list->Head, &Task1.NodeEvent);

    OS_schedulerLock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &OSstate.PendingReadyTaskList);

    OS_schedulerUnlock();

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(get_gick_count)
{
    int n = 3;

    for(int i = 0; i < 3; ++i)
    {
        BOOST_CHECK_EQUAL(OS_getTickCount(), i);

        OS_schedulerLock();
        OS_tick();
        OS_schedulerUnlock();
    }

    BOOST_CHECK_EQUAL(OS_getTickCount(), n);
}

BOOST_AUTO_TEST_SUITE_END()
