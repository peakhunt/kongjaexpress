#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <ev.h>
#include "item_pool.h"

///////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// module privats
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// buffer management
//
///////////////////////////////////////////////////////////////////////////////
static item_pool_buffer_t*
__get_item_pool_buffer(item_pool_t* pool)
{
  item_pool_buffer_t*   buf = NULL;

  pthread_mutex_lock(&pool->lock);

  if(!list_empty(&pool->free_buffer_list))
  {
    buf = list_first_entry(&pool->free_buffer_list, item_pool_buffer_t, le);
    list_del_init(&buf->le);
  }

  pthread_mutex_unlock(&pool->lock);

  return buf;
}

static void
__put_item_pool_buffer_nolock(item_pool_t* pool, item_pool_buffer_t* buf)
{
  list_add_tail(&buf->le, &pool->free_buffer_list);
}

static void
__put_item_pool_buffer(item_pool_t* pool, item_pool_buffer_t* buf)
{
  pthread_mutex_lock(&pool->lock);

  __put_item_pool_buffer_nolock(pool, buf);

  pthread_mutex_unlock(&pool->lock);
}

///////////////////////////////////////////////////////////////////////////////
//
// item consumer
//
///////////////////////////////////////////////////////////////////////////////
static void
__consume_buffer(void* arg)
{
  item_pool_buffer_t*   buf = (item_pool_buffer_t*)arg;
  item_pool_t*          pool = buf->pool;

  pool->consume(buf);

  __put_item_pool_buffer(pool, buf);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
item_pool_init(task_t* task, item_pool_t* pool, int32_t num_bufs)
{
  int     i;

  pthread_mutex_init(&pool->lock, NULL);

  INIT_LIST_HEAD(&pool->free_buffer_list);

  pool->task        = task;
  pool->buffers     = (item_pool_buffer_t*)malloc(num_bufs * sizeof(item_pool_buffer_t));
  pool->num_buffers = num_bufs;

  for(i = 0; i < num_bufs; i++)
  {
    item_pool_buffer_t* buf = &pool->buffers[i];

    buf->pool   = pool;
    INIT_LIST_HEAD(&buf->le);

    __put_item_pool_buffer_nolock(pool, buf);

    pool->item_buffer_init(pool, buf, i);
  }
}

item_pool_buffer_t* item_pool_get(item_pool_t* pool)
{
  return __get_item_pool_buffer(pool);
}

void
item_pool_put(item_pool_t* pool, item_pool_buffer_t* buf)
{
  __put_item_pool_buffer(pool, buf);
}

void
item_pool_notify(item_pool_t* pool, item_pool_buffer_t* buf)
{
  task_notify(pool->task, __consume_buffer, (void*)buf);
}
