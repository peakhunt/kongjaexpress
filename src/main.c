#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"

static void t1_start(task_t* t);
static void t2_start(task_t* t);
static void t3_start(task_t* t);

typedef unsigned long ulong_t;

static ulong_t     _t1_next  = 0;
static ulong_t     _t1_expected  = 0;
static ulong_t     _t2_next  = 0;
static ulong_t     _t2_expected  = 0;

task_timer_t    _t1,
                _t2,
                _t3;

task_t      t1 =
{
  .name   =   "T1",
  .start  =   t1_start,
};

task_t      t2 =
{
  .name   =   "T2",
  .start  =   t2_start,
};

task_t      t3 =
{
  .name   =   "T3",
  .start  =   t3_start,
};

static void
message_to_t1(void* arg)
{
  ulong_t  n = (ulong_t)arg;

  if(n != _t1_expected)
  {
    log_write("T1: error, %lu, %lu\n", n, _t1_expected);
    exit(-1);
  }
  _t1_expected = n + 1;
}

static void
message_to_t2(void* arg)
{
  ulong_t  n = (ulong_t)arg;

  if(n != _t2_expected)
  {
    log_write("T2: error, %lu, %lu\n", n, _t2_expected);
    exit(-1);
  }
  _t2_expected = n + 1;
}

static void
send_message_to_t1(void)
{
  task_notify(&t1, message_to_t1, (void*)_t1_next);
  _t1_next++;
}

static void
send_message_to_t2(void)
{
  task_notify(&t2, message_to_t2, (void*)_t2_next);
  _t2_next++;
}

static void
t1_timeout (task_timer_t* t, void* arg)
{
  int i;
  //log_write("T1: %lu, %lu\n",  _t1_next, _t1_expected);
  //log_write("T1 %s\n", __func__);

  for(i = 0; i < 12310; i++)
  {
    send_message_to_t2();
  }
}

static void
t2_timeout (task_timer_t* t, void* arg)
{
  int i;

  //log_write("T2: %lu, %lu\n", _t2_next, _t2_expected);
  //log_write("T2 %s\n", __func__);
  for(i = 0; i < 4000; i++)
  {
    send_message_to_t1();
  }
}

static void
t3_timeout (task_timer_t* t, void* arg)
{
  log_write("T3: timeout...\n");
}

static void
t1_start(task_t* t)
{
  log_write("%s callback\n", __func__);

  task_timer_init(&_t1, t1_timeout, NULL);
  task_timer_start(&_t1, 0.001, 0.001);
}

static void
t2_start(task_t* t)
{
  log_write("%s callback\n", __func__);

  task_timer_init(&_t2, t2_timeout, NULL);
  task_timer_start(&_t2, 0.001, 0.001);
}

static void
t3_start(task_t* t)
{
  log_write("%s callback\n", __func__);

  task_timer_init(&_t3, t3_timeout, NULL);
  task_timer_start(&_t3, 1.000, 1.000);
}

int
main()
{
  kongja_express_init();

  log_write("initializing t1...\n");
  task_init_main(&t1);
  log_write("initializing t2...\n");
  task_init(&t2);
  log_write("initializing t3...\n");
  task_init(&t3);

  log_write("starting all tasks...\n");
  task_run(&t2);
  task_run(&t3);

  task_run_no_thread(&t1);

  return 0;
}
