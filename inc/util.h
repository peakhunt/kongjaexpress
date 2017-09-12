#ifndef __UTIL_DEF_H__
#define __UTIL_DEF_H__

#include "log.h"

#define UNUSED(__v__)       (void)__v__

#define pr_err(fmt, ...)\
  log_write("%s:%s-%d:"fmt, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

#define pr_info pr_err

#define CRASH()\
{\
  char* _zolla_ptr = NULL;\
  *_zolla_ptr = 0;\
}

#define BUG(fmt, ...)\
{\
  pr_err("crashing sw:"fmt, ##__VA_ARGS__);\
  CRASH();\
}

#endif //!__UTIL_DEF_H__
