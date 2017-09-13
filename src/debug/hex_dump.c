#include <stdio.h>
#include "common.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// debug packet dump
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
hex_dump_buffer(const char* title, const char* head, uint8_t* buf, int len)
{
  char  buffer[128];
  int   i,
        j,
        row_len,
        buf_ndx;

  log_write("===== %s, len: %d =====\n", title, len);
  for(i = 0; i < len; i += 8)
  {
    row_len = (len - i * 8);
    row_len = row_len >= 8 ? 8 : row_len;

    buf_ndx = 0;

    buf_ndx += sprintf(&buffer[buf_ndx], "%s : ", head);
    for(j = 0; j < row_len; j++)
    {
      buf_ndx += sprintf(&buffer[buf_ndx], "%02x ", buf[i * 8 + j]);
    }

    log_write("%04d: %s\n", i / 8, buffer);
  }
}

