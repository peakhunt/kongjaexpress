#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"

static task_t   _idle_task = 
{
  .name   = "Idle Task",
};

void
idle_task_run(void)
{
  task_init_main(&_idle_task);
  task_run_no_thread(&_idle_task);
}
