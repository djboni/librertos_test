#include <boost/test/unit_test.hpp>
#include "LibreRTOS.h"
#include "OSlist.h"
#include "TheHeader.h"
#include <stack>

struct TimerFixture {

    static std::stack<struct Timer_t*> timerStack;

    static void timerFunction(struct Timer_t* timer, void*)
    {
        timerStack.push(timer);
    }
    static void timerFunction_Auto(struct Timer_t* timer, void*)
    {
        Timer_stop(timer);
        timerStack.push(timer);
    }
    static void timerFunction_UpdateTick(struct Timer_t* timer, void*)
    {
        OS_tick();
        timerStack.push(timer);
    }
    static void timerFunction_ResetTimer(struct Timer_t* timer, void*)
    {
        Timer_reset(&TimerOneShot);
        timerStack.push(timer);
    }

    struct Timer_t Timer1;
    struct Timer_t Timer2;
    struct Timer_t Timer3;
    struct Timer_t TimerAuto;
    static struct Timer_t TimerOneShot;

    TimerFixture() {
        while(!timerStack.empty())
        {
            timerStack.pop();
        }

        OS_init();
        OS_start();

        OS_timerTaskCreate(1);

        Timer_init(&Timer1, TIMERTYPE_DEFAULT, 1, &timerFunction, (void*)1);
        Timer_init(&Timer2, TIMERTYPE_DEFAULT, 2, &timerFunction, (void*)2);
        Timer_init(&Timer3, TIMERTYPE_DEFAULT, 3, &timerFunction, (void*)3);
        Timer_init(&TimerAuto, TIMERTYPE_AUTO, 1, &timerFunction_Auto, (void*)10);
        Timer_init(&TimerOneShot, TIMERTYPE_ONESHOT, 0, &timerFunction, (void*)20);
    }
    ~TimerFixture() {
        BOOST_CHECK_EQUAL(OSstate.SchedulerLock, 0);
    }
};

std::stack<struct Timer_t*> TimerFixture::timerStack;
struct Timer_t TimerFixture::TimerOneShot;

BOOST_FIXTURE_TEST_SUITE(Timer, TimerFixture)

BOOST_AUTO_TEST_CASE(init)
{
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 0);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(start_stopped_timer)
{
    Timer_start(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(start_running_timer)
{
    // Manually insert timer on ordered timers list
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    Timer_start(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(reset_stopped_timer)
{
    Timer_reset(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(reset_running_timer)
{
    Timer_reset(&Timer1);
    Timer_reset(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(reset_running_index_timer)
{
    // Manually insert timer on ordered timers list
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    Timer_reset(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerUnorderedList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(stop_stopped_timer)
{
    Timer_stop(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 0);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(stop_running_timer)
{
    Timer_reset(&Timer1);
    Timer_stop(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 0);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(stop_running_index_timer)
{
    // Manually insert timer on ordered timers list
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 1);

    Timer_stop(&Timer1);
    BOOST_CHECK_EQUAL(Timer_isRunning(&Timer1), 0);

    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(run_oneshot_timer)
{
    Timer_reset(&TimerOneShot);
    BOOST_CHECK_EQUAL(Timer_isRunning(&TimerOneShot), 1);

    // Run timer task
    OS_scheduler();
    BOOST_CHECK_EQUAL(Timer_isRunning(&TimerOneShot), 0);

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 1);
    BOOST_CHECK_EQUAL(timerStack.top(), &TimerOneShot);

    // Test timer data
    BOOST_CHECK_EQUAL(TimerOneShot.Type, TIMERTYPE_ONESHOT);
    BOOST_CHECK_EQUAL(TimerOneShot.Period, 0);
    BOOST_CHECK_EQUAL(TimerOneShot.Function, &timerFunction);
    BOOST_CHECK_EQUAL(TimerOneShot.Parameter, (void*)20);

    BOOST_CHECK_EQUAL(TimerOneShot.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerOneShot.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerOneShot.NodeTimer.Value, 0);
    BOOST_CHECK_EQUAL(TimerOneShot.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerOneShot.NodeTimer.Owner, &TimerOneShot);
}

BOOST_AUTO_TEST_CASE(run_not_oneshot_timer)
{
    Timer_reset(&Timer1);

    // Run timer task
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 0);

    // Test timer data
    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(no_timer)
{
    // Run timer task
    OS_scheduler();
}

BOOST_AUTO_TEST_CASE(not_ready_timer)
{
    // Manually insert timer on ordered timers list
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;

    // Run timer task
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 0);

    // Test timer data
    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)&OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, &OSstate.TimerList);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(ready_timer)
{
    // Manually insert timer on ordered timers list
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;

    // Run timer task
    OS_tick();
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 1);
    BOOST_CHECK_EQUAL(timerStack.top(), &Timer1);

    // Test timer data
    BOOST_CHECK_EQUAL(Timer1.Type, TIMERTYPE_DEFAULT);
    BOOST_CHECK_EQUAL(Timer1.Period, 1);
    BOOST_CHECK_EQUAL(Timer1.Function, &timerFunction);
    BOOST_CHECK_EQUAL(Timer1.Parameter, (void*)1);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Owner, &Timer1);
}

BOOST_AUTO_TEST_CASE(ready_auto_timer)
{
    // Manually insert timer on ordered timers list
    TimerAuto.NodeTimer.Value = (tick_t)(OSstate.Tick + TimerAuto.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &TimerAuto.NodeTimer);
    OSstate.TimerIndex = &TimerAuto.NodeTimer;

    // Run timer task
    OS_tick();
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 1);
    BOOST_CHECK_EQUAL(timerStack.top(), &TimerAuto);

    // Test timer data
    // The timer is not in one of the timers list because it stops itself.
    BOOST_CHECK_EQUAL(TimerAuto.Type, TIMERTYPE_AUTO);
    BOOST_CHECK_EQUAL(TimerAuto.Period, 1);
    BOOST_CHECK_EQUAL(TimerAuto.Function, &timerFunction_Auto);
    BOOST_CHECK_EQUAL(TimerAuto.Parameter, (void*)10);

    BOOST_CHECK_EQUAL(TimerAuto.NodeTimer.Next, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerAuto.NodeTimer.Previous, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerAuto.NodeTimer.Value, 1);
    BOOST_CHECK_EQUAL(TimerAuto.NodeTimer.List, (void*)NULL);
    BOOST_CHECK_EQUAL(TimerAuto.NodeTimer.Owner, &TimerAuto);
}

BOOST_AUTO_TEST_CASE(change_index)
{
    OSstate.Tick = MAX_DELAY-1;

    // Manually insert timers on ordered timers list

    // Timer1 expires before Tick overflows
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;

    // Timer2 expires after Tick overflows
    Timer2.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer2.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head->Previous, &Timer2.NodeTimer);

    // Run timer task
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 0);

    // Run timer task
    OS_tick();
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 1);
    BOOST_CHECK_EQUAL(timerStack.top(), &Timer1);
    timerStack.pop();

    // Run timer task
    OS_tick();
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 1);
    BOOST_CHECK_EQUAL(timerStack.top(), &Timer2);
}

BOOST_AUTO_TEST_CASE(concurrent_tick_readies_timer)
{
    // Manually insert timer on ordered timers list

    // Timer1 expires before Tick overflows
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;

    TimerOneShot.Function = &timerFunction_UpdateTick;
    Timer_reset(&TimerOneShot);

    // Run timer task
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 2);
    BOOST_CHECK_EQUAL(timerStack.top(), &Timer1);
    timerStack.pop();
    BOOST_CHECK_EQUAL(timerStack.top(), &TimerOneShot);
}

BOOST_AUTO_TEST_CASE(concurrent_reset_timer)
{
    // Manually insert timer on ordered timers list

    // Timer1 expires before Tick overflows
    Timer1.Function = &timerFunction_ResetTimer;
    Timer1.NodeTimer.Value = (tick_t)(OSstate.Tick + Timer1.Period);
    OS_listInsertAfter(&OSstate.TimerList, OSstate.TimerList.Head, &Timer1.NodeTimer);
    OSstate.TimerIndex = &Timer1.NodeTimer;

    // Run timer task
    OS_tick();
    OS_scheduler();

    // Test timers that were ran
    BOOST_CHECK_EQUAL(timerStack.size(), 2);
    BOOST_CHECK_EQUAL(timerStack.top(), &TimerOneShot);
    timerStack.pop();
    BOOST_CHECK_EQUAL(timerStack.top(), &Timer1);
}

BOOST_AUTO_TEST_CASE(insert_3_timers_not_overflowed)
{
    Timer_reset(&Timer1);
    OS_scheduler();
    Timer_reset(&Timer3);
    OS_scheduler();
    Timer_reset(&Timer2);
    OS_scheduler();

    BOOST_CHECK_EQUAL(OSstate.TimerList.Length, 3);
    BOOST_CHECK_EQUAL(OSstate.TimerList.Head, &Timer1.NodeTimer);
    BOOST_CHECK_EQUAL(OSstate.TimerList.Tail, &Timer3.NodeTimer);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, &Timer2.NodeTimer);
    BOOST_CHECK_EQUAL(Timer2.NodeTimer.Next, &Timer3.NodeTimer);
    BOOST_CHECK_EQUAL(Timer3.NodeTimer.Next, (struct taskListNode_t*)&OSstate.TimerList);
}

BOOST_AUTO_TEST_CASE(insert_3_timers_overflowed)
{
    OSstate.Tick = MAX_DELAY;

    Timer_reset(&Timer1);
    OS_scheduler();
    Timer_reset(&Timer3);
    OS_scheduler();
    Timer_reset(&Timer2);
    OS_scheduler();

    BOOST_CHECK_EQUAL(OSstate.TimerList.Length, 3);
    BOOST_CHECK_EQUAL(OSstate.TimerList.Head, &Timer1.NodeTimer);
    BOOST_CHECK_EQUAL(OSstate.TimerList.Tail, &Timer3.NodeTimer);

    BOOST_CHECK_EQUAL(Timer1.NodeTimer.Next, &Timer2.NodeTimer);
    BOOST_CHECK_EQUAL(Timer2.NodeTimer.Next, &Timer3.NodeTimer);
    BOOST_CHECK_EQUAL(Timer3.NodeTimer.Next, (struct taskListNode_t*)&OSstate.TimerList);
}

BOOST_AUTO_TEST_SUITE_END()
