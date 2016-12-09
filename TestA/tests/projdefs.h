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
#define LIBRERTOS_MAX_PRIORITY       10  /* integer > 0 */
#define LIBRERTOS_PREEMPTION         0  /* boolean */

typedef int8_t priority_t;
typedef uint8_t schedulerLock_t;
typedef uint16_t tick_t;
#define MAX_DELAY ((tick_t)-1)

/* Assert macro. */
void myassert(int x);
#define ASSERT(x) myassert(x)

/* Enable/disable interrupts macros. */
#define INTERRUPTS_ENABLE()
#define INTERRUPTS_DISABLE()

/* Nested critical section management macros. */
#define CRITICAL_VAL()   int _cpu_state = 0
#define CRITICAL_ENTER() do{++_cpu_state;}while(0)
#define CRITICAL_EXIT()  do{--_cpu_state;}while(0)

#ifdef __cplusplus
}
#endif

#endif /* PROJDEFS_H_ */
