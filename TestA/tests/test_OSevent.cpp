#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSevent.h"

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
    OS_eventPrePendTask(&event.ListRead, task1);

    BOOST_CHECK_EQUAL(task1->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 1);

    struct task_t* task2 = NULL;
    task2 = OS_taskCreate(1, (taskFunction_t)NULL, NULL);
    OS_eventPrePendTask(&event.ListRead, task2);

    BOOST_CHECK_EQUAL(task2->NodeEvent.List, &event.ListRead);

    BOOST_CHECK_EQUAL(event.ListRead.Head, &task2->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Tail, &task1->NodeEvent);
    BOOST_CHECK_EQUAL(event.ListRead.Length, 2);
}


BOOST_AUTO_TEST_SUITE_END()
