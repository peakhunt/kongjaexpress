#ifndef __TASK_TIMER_DEF_H__
#define __TASK_TIMER_DEF_H__

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <ev.h>

#include "common.h"
#include "list.h"

struct __task_timer_t;
typedef struct __task_timer_t task_timer_t;
typedef void (*task_timer_cb)(task_timer_t* t, void* arg);

struct __task_timer_t
{
  ev_timer        ev_tmr;
  task_timer_cb   cb;
  void*           arg;
};

extern void task_timer_init(task_timer_t* t, task_timer_cb cb, void* arg);
extern void task_timer_start(task_timer_t* t, double start, double repeat);
extern void task_timer_stop(task_timer_t* t);

static inline bool
task_timer_active(task_timer_t* t)
{
  return ev_is_active(&t->ev_tmr);
}

static inline void
task_timer_restart(task_timer_t* t, double start, double repeat)
{
  if(task_timer_active(t))
  {
    task_timer_stop(t);
  }

  task_timer_start(t, start, repeat);
}


#endif /* !__TASK_TIMER_DEF_H__ */
