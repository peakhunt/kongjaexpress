#ifndef __TASK_DEF_H__
#define __TASK_DEF_H__

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <ev.h>
#include "list.h"

/*
 * less is more!!!
 *
 * this is the very basic definition of task framework.
 *
 */

struct _task_msg_t;                     /** forward declaration         */
typedef struct _task_msg_t task_msg_t;  /** typedef for task_msg_t      */


struct _task_msg_t
{
  struct list_head      le;             /** list element in task queue  */
  void (*handler)(void*);               /** generic message handler     */
  void *arg;                            /** generic parameter           */
};

typedef struct __task_queue_t
{
  struct list_head      msgs;           /** list of task messages             */
  uint32_t              num_msgs;       /** number of messages in the queue   */
  pthread_mutex_t       q_mutex;        /** task message queue mutex          */
} task_queue_t;

struct _task_t;                         /** forward declaration               */
typedef struct _task_t task_t;          /** typedef for task_t                */

struct _task_t
{
  struct list_head      le;             /** globally registered list of tasks */
  const char*           name;           /** task name                         */
  task_queue_t          task_q;         /** task message queue                */
  pthread_t             thread;         /** self thread                       */
  struct ev_loop*       ev_loop;        /** libev ev_loop for this task       */
  ev_async              q_noti;         /** task queue notification           */

  void*                 priv;           /** extension mechanism for user      */

  void (*init)(task_t* t);              /** task init callback                */
  void (*start)(task_t* t);             /** task startup callback             */
};

/*
 * task framework APIs
 */
extern void task_framework_init(void);
extern int32_t task_init(task_t* task);
extern int32_t task_init_main(task_t* task);
extern void task_run(task_t*    task);
extern void task_run_no_thread(task_t* task);
extern int32_t task_notify(task_t* task, void (*handler)(void*), void* arg);
extern task_t* task_self(void);

#endif /* !__TASK_DEF_H__ */
