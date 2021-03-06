#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

#include "common.h"
#include "util.h"
#include "serial.h"
#include "trace.h"

///////////////////////////////////////////////////////////////////////////////
//
// utilities
//
///////////////////////////////////////////////////////////////////////////////
static bool
check_serial_config(const SerialConfig* config)
{
  static const int allowed_bauds[] = 
  {
    B1200,
    B2400,
    B4800,
    B9600,
    B19200,
    B38400,
    B57600,
    B115200,
  };
  int i;

  if(!(config->data_bit == 7 || config->data_bit == 8))
  {
    TRACE(SERIAL, "invalid data_bit %d\n", config->data_bit);
    return false;
  }

  if(!(config->stop_bit == 1 || config->stop_bit == 2))
  {
    TRACE(SERIAL, "invalid stop_bit %d\n", config->stop_bit);
    return false;
  }

  for(i = 0; i < NARRAY(allowed_bauds); i++)
  {
    if(config->baud == allowed_bauds[i])
    {
      return true;
    }
  }
  TRACE(SERIAL, "invalid baud rate %d\n", config->baud);
  return false;
}

static void
set_serial_attribute(int fd, const SerialConfig* config)
{
  struct termios tty;

  tcgetattr(fd, &tty);

  tty.c_cflag       = 0;     // control option
  tty.c_iflag       = 0;     // input option
  tty.c_lflag       = 0;     // line option
  tty.c_oflag       = 0;     // output option

  // speed
  cfsetospeed(&tty, config->baud);
  cfsetispeed(&tty, config->baud);

  // databit
  tty.c_cflag |= (config->data_bit == 7 ? 0 : CS8);

  // stopbit
  tty.c_cflag |= (config->stop_bit == 1 ? 0 : CSTOPB);

  // parity
  switch(config->parity)
  {
  case SerialParity_None:
    // nothing to do
    break;

  case SerialParity_Even:
    tty.c_cflag |= PARENB;
    tty.c_iflag |= INPCK | ISTRIP;
    break;

  case SerialParity_Odd:
    tty.c_cflag |= (PARENB | PARODD);
    tty.c_iflag |= (INPCK  | ISTRIP);
    break;
  }

  tty.c_cc[VMIN]    = 1;
  tty.c_cc[VTIME]   = 0;

  if(tcsetattr(fd, TCSAFLUSH, &tty) != 0)
  {
    // XXX
    // log error
    TRACE(SERIAL, "tcsetattr failed\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
int
serial_init(const char* path, const SerialConfig* config)
{
  int   fd;

  if(check_serial_config(config) == false)
  {
    TRACE(SERIAL, "check_serial_config failed\n");
    return -1;
  }

  fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
  if(fd < 0)
  {
    TRACE(SERIAL, "open failed\n");
    return -1;
  }

  set_serial_attribute(fd, config);

  return fd;
}
