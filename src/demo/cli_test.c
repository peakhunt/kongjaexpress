#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"
#include "trace.h"

static void cli_task_start(task_t* task);

static task_t     _cli_task =
{
  .name   = "CLI Task",
  .start  = cli_task_start,
};

static void
cli_task_start(task_t* task)
{
  TRACE(MAIN, "%s cli task started\n", __func__);
  cli_init(NULL, 0);
}

void
cli_task_init(void)
{
  TRACE(MAIN, "initializing cli task...\n");
  task_init(&_cli_task);
}

void
cli_task_run(void)
{
  TRACE(MAIN, "running cli task...\n");
  task_run(&_cli_task);
}
