#ifndef __CMD_LINE_OPT_DEF_H__
#define __CMD_LINE_OPT_DEF_H__

#include "common.h"
#include <unistd.h>
#include <getopt.h>

extern void handle_cmd_line_opts(int argc, char** argv, struct option* opts,
    void (*option_handler)(int ndx, const char* arg));

#endif /* !__CMD_LINE_OPT_DEF_H__ */
