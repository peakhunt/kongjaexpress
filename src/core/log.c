#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <ev.h>
#include "list.h"
#include "item_pool.h"

///////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
///////////////////////////////////////////////////////////////////////////////
#define   LOG_BUFFER_NUM_ELEM         128         
#define   LOG_BUFFER_SIZE             256         

///////////////////////////////////////////////////////////////////////////////
//
// private prototypes
//
///////////////////////////////////////////////////////////////////////////////
static void debug_log(item_pool_buffer_t* buf);
static void debug_log_buffer_init(item_pool_t* pool, item_pool_buffer_t* buf, int32_t ndx);

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t        _debug_log_buffer[LOG_BUFFER_NUM_ELEM][LOG_BUFFER_SIZE];
static task_t         _debug_log_task = 
{
  .name = "debug_logger",
};

static item_pool_t    _debug_log_pool =
{
  .item_buffer_init  = debug_log_buffer_init,
  .consume           = debug_log,
};

///////////////////////////////////////////////////////////////////////////////
//
// log task callbacks
//
///////////////////////////////////////////////////////////////////////////////
static void
debug_log_buffer_init(item_pool_t* pool, item_pool_buffer_t* buf, int32_t ndx)
{
  buf->data = (void*)&_debug_log_buffer[ndx][0];
}

static void
debug_log(item_pool_buffer_t* buf)
{
  char* msg = (char*)buf->data;

  fprintf(stdout, "%s", (char*)msg);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
log_init(void)
{
  task_init(&_debug_log_task);
  item_pool_init(&_debug_log_task, &_debug_log_pool, LOG_BUFFER_NUM_ELEM);

  task_run(&_debug_log_task);
}

void
log_write(const char* fmt, ...)
{
  va_list               args;
  item_pool_buffer_t*   buf;
  int32_t               len;

  buf = item_pool_get(&_debug_log_pool);
  if(buf == NULL)
  {
    return;
  }

  va_start(args, fmt);
  len = vsnprintf((char*)buf->data, LOG_BUFFER_SIZE, fmt, args);
  va_end(args);

  if(len > 0 && len < LOG_BUFFER_SIZE)
  {
    item_pool_notify(&_debug_log_pool, buf);
  }
  else
  {
    item_pool_put(&_debug_log_pool, buf);
  }
}
