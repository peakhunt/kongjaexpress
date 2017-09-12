#include "cmd_line_opt.h"

void
handle_cmd_line_opts(int argc, char** argv, struct option* opts,
    void (*option_handler)(int ndx, const char* arg))
{
  int   c;
  int   option_index;

  while(1)
  {
    c = getopt_long(argc, argv, NULL, opts, &option_index);
    if(c == -1)
    {
      break;
    }

    option_handler(option_index, optarg);
  }
}
