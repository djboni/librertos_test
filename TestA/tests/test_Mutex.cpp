#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "TheHeader.h"

struct MutexFixture {
    struct Mutex_t Mtx;
    struct task_t Task1;
    struct task_t Task2;

    MutexFixture() {
        OS_init();
        OS_start();

        Mutex_init(&Mtx);

        OS_taskCreate(&Task1, 0, NULL, NULL);
        OS_taskCreate(&Task2, 1, NULL, NULL);
    }
    ~MutexFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

BOOST_FIXTURE_TEST_SUITE(Mutex, MutexFixture)

BOOST_AUTO_TEST_CASE(init_unlocked)
{
    /* Unlocked */
    BOOST_CHECK_EQUAL(Mtx.Count, 0);
    BOOST_CHECK_EQUAL(Mtx.MutexOwner, (void*)0);

    /* Event struct */
    const struct taskListNode_t* nodeHead = (struct taskListNode_t*)&Mtx.Event.ListRead;
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Head, nodeHead);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(lock)
{
    setCurrentTask(&Task1);
    BOOST_CHECK_EQUAL(Mutex_lock(&Mtx), 1);

    BOOST_CHECK_EQUAL(Mtx.Count, 1);
    BOOST_CHECK_EQUAL(Mtx.MutexOwner, &Task1);
}

BOOST_AUTO_TEST_CASE(lock_already_locked)
{
    setCurrentTask(&Task1);
    BOOST_CHECK_EQUAL(Mutex_lock(&Mtx), 1);

    setCurrentTask(&Task2);
    BOOST_CHECK_EQUAL(Mutex_lock(&Mtx), 0);

    BOOST_CHECK_EQUAL(Mtx.Count, 1);
    BOOST_CHECK_EQUAL(Mtx.MutexOwner, &Task1);
}

BOOST_AUTO_TEST_CASE(lock_recursive)
{
    setCurrentTask(&Task1);
    BOOST_CHECK_EQUAL(Mutex_lock(&Mtx), 1);
    BOOST_CHECK_EQUAL(Mutex_lock(&Mtx), 1);

    BOOST_CHECK_EQUAL(Mtx.Count, 2);
    BOOST_CHECK_EQUAL(Mtx.MutexOwner, &Task1);
}

BOOST_AUTO_TEST_CASE(unlock)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);
    Mutex_unlock(&Mtx);

    BOOST_CHECK_EQUAL(Mtx.Count, 0);
}

BOOST_AUTO_TEST_CASE(unlock_recursive)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);
    Mutex_lock(&Mtx);
    Mutex_unlock(&Mtx);

    BOOST_CHECK_EQUAL(Mtx.Count, 1);
}

BOOST_AUTO_TEST_CASE(pend_0_tick)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);

    setCurrentTask(&Task2);
    Mutex_pend(&Mtx, 0);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pend_1_tick)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);

    setCurrentTask(&Task2);
    Mutex_pend(&Mtx, 1);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, &Mtx.Event.ListRead);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(pend_on_unlocked_mutex)
{
    setCurrentTask(&Task2);
    Mutex_pend(&Mtx, 1);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pend_on_owned_mutex)
{
    setCurrentTask(&Task2);
    Mutex_lock(&Mtx);
    Mutex_pend(&Mtx, 1);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(lock_pend_1_tick)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);

    setCurrentTask(&Task2);
    BOOST_CHECK_EQUAL(Mutex_lockPend(&Mtx, 1), 0);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, &Mtx.Event.ListRead);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(lock_pend_on_unlocked_mutex)
{
    setCurrentTask(&Task2);
    BOOST_CHECK_EQUAL(Mutex_lockPend(&Mtx, 1), 1);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(unlock_unblock_task)
{
    setCurrentTask(&Task1);
    Mutex_lock(&Mtx);

    setCurrentTask(&Task2);
    Mutex_pend(&Mtx, 1);

    setCurrentTask(&Task1);
    Mutex_unlock(&Mtx);

    BOOST_CHECK_EQUAL(Task2.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task2.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Mtx.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_SUITE_END()
