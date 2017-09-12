#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "task.h"

void
kongja_express_init(void)
{
  task_framework_init();
  log_init();
}
