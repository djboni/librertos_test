#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "TheHeader.h"

struct SemaphoreFixture {
    struct Semaphore_t Sem;
    struct task_t Task;

    SemaphoreFixture() {
        OS_init();
        OS_start();
    }
    ~SemaphoreFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(Semaphore, SemaphoreFixture)

BOOST_AUTO_TEST_CASE(init_taken)
{
    const int count = 0;
    const int max = 1;

    Semaphore_init(&Sem, count, max);

    BOOST_CHECK_EQUAL(Sem.Count, count);
    BOOST_CHECK_EQUAL(Sem.Max, max);

    BOOST_CHECK_EQUAL(Semaphore_getCount(&Sem), count);
    BOOST_CHECK_EQUAL(Semaphore_getMax(&Sem), max);

    const struct taskListNode_t* nodeHead = (struct taskListNode_t*)&Sem.Event.ListRead;
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Head, nodeHead);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(init_given)
{
    const int count = 1;
    const int max = 1;

    Semaphore_init(&Sem, count, max);

    BOOST_CHECK_EQUAL(Sem.Count, count);
    BOOST_CHECK_EQUAL(Sem.Max, max);

    BOOST_CHECK_EQUAL(Semaphore_getCount(&Sem), count);
    BOOST_CHECK_EQUAL(Semaphore_getMax(&Sem), max);

    const struct taskListNode_t* nodeHead = (struct taskListNode_t*)&Sem.Event.ListRead;
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Head, nodeHead);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(take)
{
    const int count = 1;
    const int max = 1;

    Semaphore_init(&Sem, count, max);

    BOOST_CHECK_EQUAL(Semaphore_take(&Sem), 1);

    BOOST_CHECK_EQUAL(Sem.Count, 0);
    BOOST_CHECK_EQUAL(Sem.Max, max);

    BOOST_CHECK_EQUAL(Semaphore_take(&Sem), 0);

    BOOST_CHECK_EQUAL(Sem.Count, 0);
    BOOST_CHECK_EQUAL(Sem.Max, max);
}

BOOST_AUTO_TEST_CASE(give)
{
    const int count = 0;
    const int max = 1;

    Semaphore_init(&Sem, count, max);

    BOOST_CHECK_EQUAL(Semaphore_give(&Sem), 1);

    BOOST_CHECK_EQUAL(Sem.Count, 1);
    BOOST_CHECK_EQUAL(Sem.Max, max);

    BOOST_CHECK_EQUAL(Semaphore_give(&Sem), 0);

    BOOST_CHECK_EQUAL(Sem.Count, 1);
    BOOST_CHECK_EQUAL(Sem.Max, max);
}

BOOST_AUTO_TEST_CASE(pend_0_tick)
{
    const tick_t ticksToWait = 0;

    Semaphore_init(&Sem, 0, 1);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_pend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pend_1_tick)
{
    const tick_t ticksToWait = 1;

    Semaphore_init(&Sem, 0, 1);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_pend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Sem.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(pend_on_given_sem)
{
    const tick_t ticksToWait = 1;

    Semaphore_init(&Sem, 1, 1);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_pend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(give_unblock_task)
{
    const int max = 1;
    const tick_t ticksToWait = 1;

    Semaphore_init(&Sem, 0, max);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_pend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Sem.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 1);

    Semaphore_give(&Sem);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(takepend_1_tick)
{
    const tick_t ticksToWait = 1;

    Semaphore_init(&Sem, 0, 1);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_takePend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Sem.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(takepend_on_given_sem)
{
    const tick_t ticksToWait = 1;

    Semaphore_init(&Sem, 1, 1);

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Semaphore_takePend(&Sem, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Sem.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_SUITE_END()
