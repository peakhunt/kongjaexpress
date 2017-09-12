#ifndef __TRACE_DEF_H__
#define __TRACE_DEF_H__

#include "common.h"
#include "log.h"

#define TRACE_COMP(__component__)           (__TRACE__ ## __component__)
#define TRACE_ON(__component__)             trace_on(TRACE_COMP(__component__))
#define TRACE_OFF(__component__)            trace_off(TRACE_COMP(__component__))
#define TRACE_IS_ON(__component__)          trace_is_on(TRACE_COMP(__component__))

#ifdef TRACE_ENABLED

#define TRACE(__component__, fmt,...)                                                 \
  if(TRACE_IS_ON(__component__))                                                      \
  {                                                                                   \
    log_write("%s:%s:%d:"fmt, #__component__, __FILE__, __LINE__,  ##__VA_ARGS__);        \
  }

#else

#define TRACE(__component__, fmt,...)

#endif

#define __TRACE__MAIN           0
#define __TRACE__TASK           1
#define __TRACE__SOCK_ERR       2
#define __TRACE__SERIAL         3
#define __TRACE__CLI_TELNET     4
#define __TRACE__CLI_SERIAL     5
#define __TRACE__CLI            6
#define __TRACE__MB_TCP_SLAVE   7
#define __TRACE__MBAP           8
#define __TRACE_MAX             __TRACE__MBAP

#define NUM_TRACE_ARRAY         (1 + __TRACE_MAX / 32)

extern void trace_init(uint32_t* inits, int num_elem);
extern void trace_on(uint32_t);
extern void trace_off(uint32_t);
extern bool trace_is_on(uint32_t);

#endif /* !__TRACE_DEF_H__ */
