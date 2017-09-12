#include <stdlib.h>
#include "task.h"
#include "log.h"
#include "util.h"

#include "trace.h"

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static pthread_key_t      _key_self;
static LIST_HEAD(_tasks);

///////////////////////////////////////////////////////////////////////////////
//
// task queue management
//
///////////////////////////////////////////////////////////////////////////////
/**
 * task_queue_init    initialize task queue 
 * @param tq          task queue to initialize
 * @return            none
 */
static void
task_queue_init(task_queue_t* tq)
{
  INIT_LIST_HEAD(&tq->msgs);
  tq->num_msgs      = 0;

  pthread_mutex_init(&tq->q_mutex, NULL);
}

/**
 * task_queue_lock        acquire lock on task queue
 * @param tq              task queue to lock
 * @return                none
 */
static inline void
task_queue_lock(task_queue_t* tq)
{
  pthread_mutex_lock(&tq->q_mutex);
}

/**
 * task_queue_unlock      release lock on task queue
 * @param tq              task queue to unlock
 * @return                none
 */
static inline void
task_queue_unlock(task_queue_t* tq)
{
  pthread_mutex_unlock(&tq->q_mutex);
}

/*
 * task_queue_add     add a new message to task queue
 * @param tq          task queue to add message to
 * @param msg         message to add to task queue
 * @return            non
 */
static void
task_queue_add(task_queue_t* tq, task_msg_t* msg)
{
  INIT_LIST_HEAD(&msg->le);

  task_queue_lock(tq);

  list_add_tail(&msg->le, &tq->msgs);
  tq->num_msgs += 1;

  task_queue_unlock(tq);
}

///////////////////////////////////////////////////////////////////////////////
//
// task queue notification handler
//
///////////////////////////////////////////////////////////////////////////////
/**
 * task_queue_noti_callback   a callback for task queue add event
 *                            beware that add events are compressed, that is,
 *                            multiple add events can be compressed into single event.
 */
static void
task_queue_noti_callback (EV_P_ ev_async *w, int revents)
{
  task_msg_t*     p;
  task_t*         self = task_self();
  LIST_HEAD(tmp_list);

  // lock task queue and read all messages or we will lose some messages
  task_queue_lock(&self->task_q);

  list_cut_position(&tmp_list, &self->task_q.msgs, self->task_q.msgs.prev);
  self->task_q.num_msgs = 0;

  task_queue_unlock(&self->task_q);

  // now run it
  while(!list_empty(&tmp_list))
  {
    p = list_first_entry(&tmp_list, task_msg_t, le);

    list_del_init(&p->le);
    p->handler(p->arg);

    free(p);
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// generic task loop
//
///////////////////////////////////////////////////////////////////////////////
/**
 * __task_start_routine     POSIX thread entry point
 *                          and contains generic mainloop
 */
static void*
__task_start_routine(void* arg)
{
  task_t*   self = (task_t*)arg;

  TRACE(TASK, "%s task started\n", self->name);

  // set TLS for task_self()
  pthread_setspecific(_key_self, (void*)self);
  ev_ref (self->ev_loop);

  if(self->start != NULL)
  {
    self->start(self);
  }

  while(1)
  {
    ev_run(self->ev_loop, 0);
  }
  TRACE(TASK,"%s task exiting\n", self->name);
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
///////////////////////////////////////////////////////////////////////////////
/**
 * task_prepare_loop      prepare libev loop before executing
 * @param task            task to initialize libev with
 * @param loop            ev_loop to use
 * @return                non
 */
static void
task_prepare_loop(task_t* task, struct ev_loop* loop)
{
  task->ev_loop = loop;
  ev_async_init(&task->q_noti, task_queue_noti_callback);
  ev_async_start(task->ev_loop, &task->q_noti);
}

/**
 * __task_init    task init helper. just initializes task structure appropriately
 * @param task    task to initialize
 * @param loop    libev loop to use
 * @return        none
 */
static void
__task_init(task_t* task, struct ev_loop* loop)
{
  INIT_LIST_HEAD(&task->le);
  list_add_tail(&task->le, &_tasks);

  task_queue_init(&task->task_q);
  task_prepare_loop(task, loop);

  if(task->init != NULL)
  {
    task->init(task);
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
/**
 * task_framework_init    initialize task framework before using it
 */
void
task_framework_init(void)
{
  pthread_key_create(&_key_self, NULL);
}

/**
 * task_init      initialize task before running it
 * 
 * @param task    task to initialize
 * @return 0 on success, -1 on failure
 */
int32_t
task_init(task_t*   task)
{
  __task_init(task, ev_loop_new(0));
  return 0;
}

/**
 * task_init_main     initialize task with default ev mainloop
 *
 * @param task        task to initialize
 * @return 0 on success, -1 on failure
 */
int32_t
task_init_main(task_t* task)
{
  __task_init(task, EV_DEFAULT);
  return 0;
}

/**
 * task_run       run given task
 * 
 * @param task    task to run
 * @return        none 
 */
void
task_run(task_t*    task)
{
  pthread_create(&task->thread, NULL, __task_start_routine, (void*)task);
}

/**
 * task_run_no_thread     run task without creating thread
 *
 * @param task            task to run
 * @return                none
 */
void
task_run_no_thread(task_t* task)
{
  __task_start_routine((void*)task);
}

/**
 * task_notify    send a message to task and wake it up
 * 
 * @param task    task to send message to
 * @param msg     message to send
 *
 * @return        0 on success, -1 on failure 
 */
int32_t
task_notify(task_t* task, void (*handler)(void*), void* arg)
{
  task_msg_t*   m = malloc(sizeof(task_msg_t));

  if(m == NULL)
  {
    return -1;
  }

  m->handler  = handler;
  m->arg      = arg;

  task_queue_add(&task->task_q, m);
  ev_async_send(task->ev_loop, &task->q_noti);
  return 0;
}

/**
 * task_self      get self task pointer in a thread
 * @return        task_t pointer of self task
 */
task_t*
task_self(void)
{
  task_t* self;

  self = (task_t*)pthread_getspecific(_key_self);
  return self;
}
