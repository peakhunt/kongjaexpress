#ifndef __ITEM_POOL_DEF_H__
#define __ITEM_POOL_DEF_H__

#include "list.h"
#include "task.h"

struct __item_pool_t;
typedef struct __item_pool_t item_pool_t;

typedef struct
{
  struct list_head    le;
  item_pool_t*        pool;
  void*               data;
} item_pool_buffer_t;

struct __item_pool_t
{
  task_t*             task;
  pthread_mutex_t     lock;
  item_pool_buffer_t* buffers;
  int32_t             num_buffers;
  struct list_head    free_buffer_list;

  void (*item_buffer_init)(item_pool_t* pool, item_pool_buffer_t* buf, int32_t ndx);
  void (*consume)(item_pool_buffer_t* buf);
};

extern void item_pool_init(task_t* task, item_pool_t* pool, int32_t num_bufs);
extern item_pool_buffer_t* item_pool_get(item_pool_t* pool);
extern void item_pool_put(item_pool_t* pool, item_pool_buffer_t* buf);
extern void item_pool_notify(item_pool_t* pool, item_pool_buffer_t* buf);

#endif /* !__ITEM_POOL_DEF_H__ */
