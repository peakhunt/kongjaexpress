#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#include <stdint.h>

typedef uint8_t     bool;

#define true        1
#define false       0

#define TRUE        true
#define FALSE       false

#define NARRAY(a)     (sizeof(a)/sizeof(a[0]))
#define UNUSED(a)     (void)(a)

#endif /* !__COMMON_DEF_H__ */
