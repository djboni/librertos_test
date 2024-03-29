/*
 Copyright 2016 Djones A. Boni

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#ifndef PROJDEFS_H_
#define PROJDEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdint.h>

/* LibreRTOS definitions. */
#define LIBRERTOS_MAX_PRIORITY 10  /* integer > 0 */
#define LIBRERTOS_PREEMPTION 0     /* boolean */
#define LIBRERTOS_PREEMPT_LIMIT 0  /* integer >= 0, < LIBRERTOS_MAX_PRIORITY */
#define LIBRERTOS_SOFTWARETIMERS 1 /* boolean */
#define LIBRERTOS_STATE_GUARDS 0   /* boolean */
#define LIBRERTOS_STATISTICS 0     /* boolean */

typedef int8_t priority_t;
typedef uint8_t schedulerLock_t;
typedef uint16_t tick_t;
typedef int16_t difftick_t;
typedef uint32_t stattime_t;
typedef int16_t len_t;
typedef uint8_t bool_t;

#define MAX_DELAY ((tick_t)-1)

/* Assert macro. */
void myassert(int x);
#define ASSERT(x) myassert(x)

/* Enable/disable interrupts macros. */
#define INTERRUPTS_ENABLE()                                                    \
  do {                                                                         \
    volatile int x = 0;                                                        \
    ++x;                                                                       \
  } while (0)
#define INTERRUPTS_DISABLE()                                                   \
  do {                                                                         \
    volatile int x = 0;                                                        \
    ++x;                                                                       \
  } while (0)

/* Nested critical section management macros. */
#define CRITICAL_VAL() int _cpu_state = 0
#define CRITICAL_ENTER()                                                       \
  do {                                                                         \
    ++_cpu_state;                                                              \
  } while (0)
#define CRITICAL_EXIT()                                                        \
  do {                                                                         \
    --_cpu_state;                                                              \
  } while (0)

/* Simulate concurrent access. For test coverage only. */
void librertos_test_set_concurrent_behavior(void (*f)(void));
void librertos_test_concurrent_access(void);
#define LIBRERTOS_TEST_CONCURRENT_ACCESS() librertos_test_concurrent_access()

#ifdef __cplusplus
}
#endif

#endif /* PROJDEFS_H_ */
