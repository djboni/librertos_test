#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSlist.h"
#include "TheHeader.h"

struct test_func__OS_taskDelay__Fixture {
    struct task_t Task1;

    test_func__OS_taskDelay__Fixture() {
        OS_init();
        OS_start();
    }
    ~test_func__OS_taskDelay__Fixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(func__OS_taskDelay, test_func__OS_taskDelay__Fixture)

BOOST_AUTO_TEST_CASE(delay_0_tick)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_taskDelay(0);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(delay_not_overflowed)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_taskDelay(1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
}

BOOST_AUTO_TEST_CASE(delay_max_ticks)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_schedulerLock();
    OS_tick();
    OS_schedulerUnlock();

    OS_taskDelay(MAX_DELAY);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList2);
}

BOOST_AUTO_TEST_CASE(resume_blocked_task)
{
    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_taskDelay(1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);

    OS_taskResume(&Task1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
}

BOOST_AUTO_TEST_CASE(resume_suspended_task)
{
    struct taskHeadList_t list;

    OS_listHeadInit(&list);

    OS_taskCreate(&Task1, 0, 0, 0);
    setCurrentTask(&Task1);

    OS_listInsertAfter(&list, list.Head, &Task1.NodeEvent);

    OS_taskDelay(1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_BLOCKED);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &list);

    OS_taskResume(&Task1);

    BOOST_CHECK_EQUAL(Task1.State, TASKSTATE_READY);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
}

BOOST_AUTO_TEST_SUITE_END()
