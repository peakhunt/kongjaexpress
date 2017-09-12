#include "request_queue.h"


void
request_queue_init(request_queue_t* q)
{
  for(int i = 0; i < request_priority_max; i++)
  {
    INIT_LIST_HEAD(&q->queue[i]);
  }

  q->num_reqs = 0;
}

void
request_queue_submit(request_queue_t* q, request_item_t* item)
{
  list_add_tail(&item->le, &q->queue[item->priority]);
  q->num_reqs++;
}

void
request_queue_schedule(request_queue_t* q)
{
  request_item_t* item;

  for(int i = 0; i < request_priority_max; i++)
  {
    if(!list_empty(&q->queue[i]))
    {
      item = list_first_entry(&q->queue[i], request_item_t, le);
      list_del_init(&item->le);

      q->request_accepted(q, item);
      q->num_reqs--;
      return;
    }
  }
}

void
reqiest_queue_reset(request_queue_t* q)
{
  request_item_t  *p, *n;

  for(int i = 0; i < request_priority_max; i++)
  {
    list_for_each_entry_safe(p, n, &q->queue[i], le)
    {
      list_del_init(&p->le);
      if(q->request_cancelled)
      {
        q->request_cancelled(q, p);
      }
      q->num_reqs--;
    }
  }
}
