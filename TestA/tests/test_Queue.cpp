#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "TheHeader.h"
#include <random>

typedef uint64_t QueType;

struct QueueFixture {
    static const int QueSize = 3;
    static const QueType QueGuard = 0xFA57C0DEFA57C0DE;

    const int Len = QueSize;

    /* Buffer with guards at begin and end. */
    const QueType Guard = QueGuard;
    QueType QueBuff_WithGuards[QueSize + 2];

    struct Queue_t Que;
    QueType* QueBuff = &QueBuff_WithGuards[1];

    struct task_t Task;

    QueueFixture() {
        OS_init();
        OS_start();

        QueBuff_WithGuards[0] = Guard;
        QueBuff_WithGuards[QueSize+1] = Guard;

        Queue_init(&Que, &QueBuff[0], QueSize, sizeof(QueType));
    }
    ~QueueFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);

        /* These should not change after initialization. */
        BOOST_CHECK_EQUAL(Queue_length(&Que), Len);
        BOOST_CHECK_EQUAL(Queue_itemSize(&Que), sizeof(QueType));

        /* These should be zero after operations are complete. */
        BOOST_CHECK_EQUAL(Que.WLock, 0);
        BOOST_CHECK_EQUAL(Que.RLock, 0);

        /* Constants after initialization. */
        BOOST_CHECK_EQUAL(Que.ItemSize, sizeof(QueType));
        BOOST_CHECK_EQUAL(Que.Buff, (void*)&QueBuff[0]);
        BOOST_CHECK_EQUAL(Que.BufEnd, (void*)&QueBuff[Len-1]);

        /* Check overflow guards. */
        BOOST_CHECK_EQUAL(QueBuff_WithGuards[0], Guard);
        BOOST_CHECK_EQUAL(QueBuff_WithGuards[QueSize+1], Guard);
    }
};

BOOST_FIXTURE_TEST_SUITE(Queue, QueueFixture)

BOOST_AUTO_TEST_CASE(init)
{
    BOOST_CHECK_EQUAL(Que.ItemSize, sizeof(QueType));
    BOOST_CHECK_EQUAL(Que.Free, Len);
    BOOST_CHECK_EQUAL(Que.Used, 0);
    BOOST_CHECK_EQUAL(Que.WLock, 0);
    BOOST_CHECK_EQUAL(Que.RLock, 0);
    BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.Buff, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.BufEnd, (void*)&QueBuff[Len-1]);

    struct taskListNode_t* nodeHead;

    nodeHead = (struct taskListNode_t*)&Que.Event.ListRead;
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Head, nodeHead);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 0);

    nodeHead = (struct taskListNode_t*)&Que.Event.ListWrite;
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Head, nodeHead);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(write)
{
    QueType x;
    std::srand(0);

    for(int i = 0; i < Len; ++i)
    {
        x = std::rand();

        BOOST_CHECK_EQUAL(Queue_write(&Que, &x), 1);

        BOOST_CHECK_EQUAL(Que.Free, Len-(i+1));
        BOOST_CHECK_EQUAL(Que.Used, i+1);

        BOOST_CHECK_EQUAL(Queue_free(&Que), Len-(i+1));
        BOOST_CHECK_EQUAL(Queue_used(&Que), i+1);

        BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[0]);
        BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[i+1 >= Len ? i+1-Len : i+1]);
        BOOST_CHECK_EQUAL(QueBuff[i], x);
    }

    x = std::rand();
    BOOST_CHECK_EQUAL(Queue_write(&Que, &x), 0);

    BOOST_CHECK_EQUAL(Que.Free, 0);
    BOOST_CHECK_EQUAL(Que.Used, Len);
    BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[0]);
}

struct Queue_t* queueToWrite = NULL;

void write_to_queue(void)
{
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(queueToWrite->WLock, 1);

    QueType x = std::rand();
    BOOST_CHECK_EQUAL(Queue_write(queueToWrite, &x), 1);

    BOOST_CHECK_EQUAL(queueToWrite->WLock, 2);
}

BOOST_AUTO_TEST_CASE(write_concurrent)
{
    QueType x;
    std::srand(0);

    librertos_test_set_concurrent_behavior(&write_to_queue);
    queueToWrite = &Que;

    x = std::rand();
    BOOST_CHECK_EQUAL(Queue_write(&Que, &x), 1);

    queueToWrite = NULL;
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(Que.Free, Len-2);
    BOOST_CHECK_EQUAL(Que.Used, 2);
    BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[2]);
}

void queueFill(struct Queue_t& Que)
{
    std::srand(0);
    for(;;)
    {
        QueType x = std::rand();
        if(Queue_write(&Que, &x) == 0)
            break;
    }
}

BOOST_AUTO_TEST_CASE(read)
{
    queueFill(Que);

    QueType x;
    std::srand(0);

    for(int i = 0; i < Len; ++i)
    {
        BOOST_CHECK_EQUAL(Queue_read(&Que, &x), 1);

        BOOST_CHECK_EQUAL(x, std::rand());

        BOOST_CHECK_EQUAL(Que.Free, i+1);
        BOOST_CHECK_EQUAL(Que.Used, Len-(i+1));

        BOOST_CHECK_EQUAL(Queue_free(&Que), i+1);
        BOOST_CHECK_EQUAL(Queue_used(&Que), Len-(i+1));

        BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[i+1 >= Len ? i+1-Len : i+1]);
        BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[0]);
        BOOST_CHECK_EQUAL(QueBuff[i], x);
    }

    x = std::rand();
    BOOST_CHECK_EQUAL(Queue_read(&Que, &x), 0);

    BOOST_CHECK_EQUAL(Que.Free, Len);
    BOOST_CHECK_EQUAL(Que.Used, 0);
    BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[0]);
    BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[0]);
}

struct Queue_t* queueToRead = NULL;

void read_from_queue(void)
{
    librertos_test_set_concurrent_behavior(0);
    QueType x;

    BOOST_CHECK_EQUAL(queueToRead->RLock, 1);

    BOOST_CHECK_EQUAL(Queue_read(queueToRead, &x), 1);

    BOOST_CHECK_EQUAL(queueToRead->RLock, 2);

    /* Second rand number. */
    std::srand(0);
    std::rand();
    BOOST_CHECK_EQUAL(x, std::rand());
}

BOOST_AUTO_TEST_CASE(read_concurrent)
{
    queueFill(Que);

    QueType x;

    librertos_test_set_concurrent_behavior(&read_from_queue);
    queueToRead = &Que;

    BOOST_CHECK_EQUAL(Queue_read(&Que, &x), 1);

    /* First rand number. */
    std::srand(0);
    BOOST_CHECK_EQUAL(x, std::rand());

    queueToWrite = NULL;
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(Que.Free, 2);
    BOOST_CHECK_EQUAL(Que.Used, Len-2);
    BOOST_CHECK_EQUAL(Que.Head, (void*)&QueBuff[2]);
    BOOST_CHECK_EQUAL(Que.Tail, (void*)&QueBuff[0]);
}

BOOST_AUTO_TEST_CASE(pendread_0_tick)
{
    const tick_t ticksToWait = 0;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendRead(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendread_1_tick)
{
    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendRead(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(pendread_on_not_empty_queue)
{
    queueFill(Que);

    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendRead(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendwrite_0_tick)
{
    queueFill(Que);

    const tick_t ticksToWait = 0;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendWrite(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendwrite_1_tick)
{
    queueFill(Que);

    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendWrite(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 1);
}

BOOST_AUTO_TEST_CASE(pendwrite_on_not_full_queue)
{
    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendWrite(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(write_unblock_task)
{
    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendRead(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 1);

    std::srand(0);
    QueType x = std::rand();
    BOOST_CHECK_EQUAL(Queue_write(&Que, &x), 1);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(read_unblock_task)
{
    queueFill(Que);

    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);
    Queue_pendWrite(&Que, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 1);

    QueType x;
    BOOST_CHECK_EQUAL(Queue_read(&Que, &x), 1);
    std::srand(0);
    BOOST_CHECK_EQUAL(x, std::rand());

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(readpend_1_tick)
{
    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);

    QueType x;
    Queue_readPend(&Que, &x, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListRead);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(readpend_on_not_empty_queue)
{
    queueFill(Que);

    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);

    QueType x;
    Queue_readPend(&Que, &x, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(writepend_1_tick)
{
    queueFill(Que);

    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);

    std::srand(0);
    QueType x = std::rand();
    Queue_writePend(&Que, &x, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, &Que.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 1);
}

BOOST_AUTO_TEST_CASE(writepend_on_not_full_queue)
{
    const tick_t ticksToWait = 1;

    OS_taskCreate(&Task, 0, NULL, NULL);
    setCurrentTask(&Task);

    std::srand(0);
    QueType x = std::rand();
    Queue_writePend(&Que, &x, ticksToWait);

    BOOST_CHECK_EQUAL(Task.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Que.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_SUITE_END()
