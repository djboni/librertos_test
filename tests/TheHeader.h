#include "LibreRTOS.h"

inline void setCurrentTask(struct task_t *task) { OSstate.CurrentTCB = task; }
