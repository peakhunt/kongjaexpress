#include "task.h"
#include "task_timer.h"

///////////////////////////////////////////////////////////////////////////////
//
// common ev timer callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__common_ev_tmr_cb(EV_P_ ev_timer* w, int revents)
{
  task_timer_t*   t = container_of(w, task_timer_t, ev_tmr);

  t->cb(t, t->arg);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
task_timer_init(task_timer_t* t, 
    task_timer_cb cb, void* arg)
{
  ev_timer_init(&t->ev_tmr, __common_ev_tmr_cb, 0, 0);

  t->cb   = cb;
  t->arg  = arg;
}

void
task_timer_start(task_timer_t* t, double start, double repeat)
{
  task_t* self = task_self();

  ev_timer_set(&t->ev_tmr, start, repeat);
  ev_timer_start(self->ev_loop, &t->ev_tmr);
}

void
task_timer_stop(task_timer_t* t)
{
  task_t* self = task_self();

  ev_timer_stop(self->ev_loop, &t->ev_tmr);
}
