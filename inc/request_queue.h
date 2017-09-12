#ifndef __REQUEST_QUEUE_DEF_H__
#define __REQUEST_QUEUE_DEF_H__

#include <stdio.h>
#include "list.h"

typedef enum
{
  request_priority_0,
  request_priority_1,
  request_priority_2,
  request_priority_3,
  request_priority_4,
  request_priority_5,
  request_priority_max,
} request_priority_t;

typedef struct __request_item
{
  struct list_head    le;
  request_priority_t  priority;
} request_item_t;

struct __request_queue;
typedef struct __request_queue request_queue_t;

struct __request_queue
{
  struct list_head    queue[request_priority_max];
  int                 num_reqs;

  void (*request_accepted)(request_queue_t* q, request_item_t* item);
  void (*request_cancelled)(request_queue_t* q, request_item_t* item);
};

extern void request_queue_init(request_queue_t* q);
extern void request_queue_submit(request_queue_t* q, request_item_t* item);
extern void request_queue_schedule(request_queue_t* q);
extern void reqiest_queue_reset(request_queue_t* q);



#endif /* !__REQUEST_QUEUE_DEF_H__ */
