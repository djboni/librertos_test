#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "TheHeader.h"
#include <random>

typedef uint64_t FifType;

struct FifoFixture {
    static const int FifSize = 1;
    static const FifType FifGuard = 0xFA57C0DEFA57C0DE;

    const int Len = FifSize * (int)sizeof(FifType);

    /* Buffer with guards at begin and end. */
    const FifType Guard = FifGuard;
    FifType FifBuff_WithGuards[FifSize + 2];

    struct Fifo_t Fif;
    uint8_t* FifBuff = (uint8_t*)&FifBuff_WithGuards[1];

    struct task_t Task1;

    FifoFixture() {
        OS_init();
        OS_start();

        FifBuff_WithGuards[0] = Guard;
        FifBuff_WithGuards[FifSize+1] = Guard;

        Fifo_init(&Fif, &FifBuff[0], (uint8_t)Len);

        OS_taskCreate(&Task1, 0, NULL, NULL);
    }
    ~FifoFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);

        /* These should be zero after operations are complete. */
        BOOST_CHECK_EQUAL(Fif.WLock, 0);
        BOOST_CHECK_EQUAL(Fif.RLock, 0);

        /* Constants after initialization. */
        BOOST_CHECK_EQUAL(Fif.Length, Len);
        BOOST_CHECK_EQUAL(Fif.Buff, (void*)&FifBuff[0]);
        BOOST_CHECK_EQUAL(Fif.BufEnd, (void*)&FifBuff[Len-1]);

        /* Check overflow guards. */
        BOOST_CHECK_EQUAL(FifBuff_WithGuards[0], Guard);
        BOOST_CHECK_EQUAL(FifBuff_WithGuards[FifSize+1], Guard);
    }
};

static uint8_t myrand()
{
    return (uint8_t)(std::rand() & 0xFF);
}

static void mysrand(unsigned int seed)
{
    std::srand(seed);
}

BOOST_FIXTURE_TEST_SUITE(Fifo, FifoFixture)

BOOST_AUTO_TEST_CASE(init)
{
    BOOST_CHECK_EQUAL(Fif.Length, Len);
    BOOST_CHECK_EQUAL(Fif.Free, Len);
    BOOST_CHECK_EQUAL(Fif.Used, 0);
    BOOST_CHECK_EQUAL(Fif.WLock, 0);
    BOOST_CHECK_EQUAL(Fif.RLock, 0);
    BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL(Fif.Buff, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL((void*)Fif.BufEnd, (void*)&FifBuff[Len-1]);

    struct taskListNode_t* nodeHead;

    nodeHead = (struct taskListNode_t*)&Fif.Event.ListRead;
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Head, nodeHead);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);

    nodeHead = (struct taskListNode_t*)&Fif.Event.ListWrite;
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Head, nodeHead);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Tail, nodeHead);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(write)
{
    uint8_t x;
    mysrand(0);

    for(int i = 0; i < Len; ++i)
    {
        x = myrand();

        BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);

        BOOST_CHECK_EQUAL(Fif.Free, Len-(i+1));
        BOOST_CHECK_EQUAL(Fif.Used, i+1);

        BOOST_CHECK_EQUAL(Fifo_free(&Fif), Len-(i+1));
        BOOST_CHECK_EQUAL(Fifo_used(&Fif), i+1);

        BOOST_CHECK_EQUAL(Fif.Head, (void*)&((uint8_t*)FifBuff)[0]);
        BOOST_CHECK_EQUAL(Fif.Tail, (void*)&((uint8_t*)FifBuff)[i+1 >= Len ? i+1-Len : i+1]);
        BOOST_CHECK_EQUAL(FifBuff[i], x);
    }

    x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 0);

    BOOST_CHECK_EQUAL(Fif.Free, 0);
    BOOST_CHECK_EQUAL(Fif.Used, Len);
    BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[0]);
}

struct Fifo_t* fifoToWrite = NULL;

void write_to_fifo(void)
{
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(fifoToWrite->WLock, 1);

    uint8_t x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(fifoToWrite, &x, 1), 1);

    BOOST_CHECK_EQUAL(fifoToWrite->WLock, 2);
}

BOOST_AUTO_TEST_CASE(write_concurrent)
{
    uint8_t x;
    mysrand(0);

    librertos_test_set_concurrent_behavior(&write_to_fifo);
    fifoToWrite = &Fif;

    x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);

    fifoToWrite = NULL;
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(Fif.Free, Len-2);
    BOOST_CHECK_EQUAL(Fif.Used, 2);
    BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[2]);
}

void fifoFill(struct Fifo_t& Fif)
{
    mysrand(0);
    for(;;)
    {
        uint8_t x = myrand();
        if(Fifo_write(&Fif, &x, 1) == 0)
            break;
    }
}

BOOST_AUTO_TEST_CASE(read)
{
    fifoFill(Fif);

    uint8_t x;
    mysrand(0);

    for(int i = 0; i < Len; ++i)
    {
        BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);

        BOOST_CHECK_EQUAL(x, myrand());

        BOOST_CHECK_EQUAL(Fif.Free, i+1);
        BOOST_CHECK_EQUAL(Fif.Used, Len-(i+1));

        BOOST_CHECK_EQUAL(Fifo_free(&Fif), i+1);
        BOOST_CHECK_EQUAL(Fifo_used(&Fif), Len-(i+1));

        BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[i+1 >= Len ? i+1-Len : i+1]);
        BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[0]);
        BOOST_CHECK_EQUAL(FifBuff[i], x);
    }

    x = myrand();
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 0);

    BOOST_CHECK_EQUAL(Fif.Free, Len);
    BOOST_CHECK_EQUAL(Fif.Used, 0);
    BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[0]);
    BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[0]);
}

struct Fifo_t* fifoToRead = NULL;

void read_from_fifo(void)
{
    librertos_test_set_concurrent_behavior(0);
    uint8_t x;

    BOOST_CHECK_EQUAL(fifoToRead->RLock, 1);

    BOOST_CHECK_EQUAL(Fifo_read(fifoToRead, &x, 1), 1);

    BOOST_CHECK_EQUAL(fifoToRead->RLock, 2);

    /* Second rand number. */
    mysrand(0);
    myrand();
    BOOST_CHECK_EQUAL(x, myrand());
}

BOOST_AUTO_TEST_CASE(read_concurrent)
{
    fifoFill(Fif);

    uint8_t x;

    librertos_test_set_concurrent_behavior(&read_from_fifo);
    fifoToRead = &Fif;

    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);

    /* First rand number. */
    mysrand(0);
    BOOST_CHECK_EQUAL(x, myrand());

    fifoToWrite = NULL;
    librertos_test_set_concurrent_behavior(0);

    BOOST_CHECK_EQUAL(Fif.Free, 2);
    BOOST_CHECK_EQUAL(Fif.Used, Len-2);
    BOOST_CHECK_EQUAL(Fif.Head, (void*)&FifBuff[2]);
    BOOST_CHECK_EQUAL(Fif.Tail, (void*)&FifBuff[0]);
}

BOOST_AUTO_TEST_CASE(write_overlapping)
{
    mysrand(0);

    // Initialization
    uint8_t x;
    for(int i = 0; i < Len-1; ++i)
    {
        x = myrand();
        BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);
    }

    mysrand(0);
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
    BOOST_CHECK_EQUAL(x, myrand());
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
    BOOST_CHECK_EQUAL(x, myrand());

    uint8_t xx[4];
    for(unsigned i = 0; i < sizeof(xx); ++i)
        xx[i] = (uint8_t)(i+1);
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &xx, sizeof(xx)), 3);

    // Written above
    BOOST_CHECK_EQUAL(FifBuff[Len-1], 1);
    BOOST_CHECK_EQUAL(FifBuff[0], 2);
    BOOST_CHECK_EQUAL(FifBuff[1], 3);

    // Written on initialization
    BOOST_CHECK_EQUAL(FifBuff[2], myrand());
}

BOOST_AUTO_TEST_CASE(read_overlapping)
{
    fifoFill(Fif);

    mysrand(0);
    uint8_t x;
    for(unsigned i = 0; i < (unsigned)(Len-1); ++i)
    {
        BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
        BOOST_CHECK_EQUAL(x, myrand());
    }
    uint8_t nextRead = myrand();

    uint8_t xx[3];
    for(unsigned i = 0; i < sizeof(xx); ++i)
        xx[i] = (uint8_t)(i+1);
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &xx, sizeof(xx)), 3);

    BOOST_CHECK_EQUAL(FifBuff[Len-1], nextRead);
    BOOST_CHECK_EQUAL(FifBuff[0], 1);
    BOOST_CHECK_EQUAL(FifBuff[1], 2);
    BOOST_CHECK_EQUAL(FifBuff[2], 3);

    uint8_t yy[5];
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &yy, sizeof(yy)), 4);

    BOOST_CHECK_EQUAL(yy[0], nextRead);
    BOOST_CHECK_EQUAL(yy[1], 1);
    BOOST_CHECK_EQUAL(yy[2], 2);
    BOOST_CHECK_EQUAL(yy[3], 3);
}

BOOST_AUTO_TEST_CASE(pendread_0_tick)
{
    const tick_t ticksToWait = 0;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendread_0_length)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 0;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendread_1_tick)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListRead);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(pendread_on_not_empty_fifo)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendwrite_0_tick)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 0;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendwrite_0_length)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 0;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(pendwrite_1_tick)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 1);
}

BOOST_AUTO_TEST_CASE(pendwrite_on_not_full_fifo)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(write_unblock_task)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListRead);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 1);

    mysrand(0);
    uint8_t x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(write_unblock_task_length2)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 2;

    setCurrentTask(&Task1);
    Fifo_pendRead(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListRead);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 1);

    mysrand(0);
    uint8_t x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListRead);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 1);

    x = myrand();
    BOOST_CHECK_EQUAL(Fifo_write(&Fif, &x, 1), 1);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(read_unblock_task)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 1);

    uint8_t x;
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
    mysrand(0);
    BOOST_CHECK_EQUAL(x, myrand());

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(read_unblock_task_length2)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 2;

    setCurrentTask(&Task1);
    Fifo_pendWrite(&Fif, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 1);

    uint8_t x;
    mysrand(0);
    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
    BOOST_CHECK_EQUAL(x, myrand());

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 1);

    BOOST_CHECK_EQUAL(Fifo_read(&Fif, &x, 1), 1);
    BOOST_CHECK_EQUAL(x, myrand());

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_CASE(readpend_1_tick)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);

    uint8_t x;
    Fifo_readPend(&Fif, &x, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListRead);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 1);
}

BOOST_AUTO_TEST_CASE(readpend_on_not_empty_fifo)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);

    uint8_t x;
    Fifo_readPend(&Fif, &x, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListRead.Length, 0);
}

BOOST_AUTO_TEST_CASE(writepend_1_tick)
{
    fifoFill(Fif);

    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);

    mysrand(0);
    uint8_t x = myrand();
    Fifo_writePend(&Fif, &x, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, &Fif.Event.ListWrite);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, &OSstate.BlockedTaskList1);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 1);
}

BOOST_AUTO_TEST_CASE(writepend_on_not_full_fifo)
{
    const tick_t ticksToWait = 1;
    const uint8_t length = 1;

    setCurrentTask(&Task1);

    mysrand(0);
    uint8_t x = myrand();
    Fifo_writePend(&Fif, &x, length, ticksToWait);

    BOOST_CHECK_EQUAL(Task1.NodeEvent.List, (void*)0);
    BOOST_CHECK_EQUAL(Task1.NodeDelay.List, (void*)0);
    BOOST_CHECK_EQUAL(Fif.Event.ListWrite.Length, 0);
}

BOOST_AUTO_TEST_SUITE_END()
