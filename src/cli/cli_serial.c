#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/telnet.h>

#include "common.h"
#include "cli.h"
#include "stream.h"
#include "serial.h"
#include "log.h"
#include "telnet_reader.h"
#include "trace.h"

typedef struct
{
  stream_t          stream;
  cli_intf_t        cli_intf;
  uint8_t           rx_buffer[128];
} cli_serial_if_t;


static cli_serial_if_t    _serial_if1;


static void
handle_stream_event(stream_t* stream, stream_event_t evt)
{
	cli_serial_if_t* intf = container_of(stream, cli_serial_if_t, stream);

	switch(evt)
  {
  case stream_event_rx:
    cli_handle_rx(&intf->cli_intf, stream->rx_buf, stream->rx_data_len);
    break;

  case stream_event_eof:
    TRACE(CLI_SERIAL, "connection eof detected\n");
    break;

  case stream_event_err:
    TRACE(CLI_SERIAL, "connection error detected\n");
    break;

  case stream_event_tx:
    // never occurs
    break;
  }
}

static void
cli_put_tx_data(cli_intf_t* cintf, char* data, int len)
{
	cli_serial_if_t* intf = container_of(cintf, cli_serial_if_t, cli_intf);

  stream_write(&intf->stream, (uint8_t*)data, len);
}

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_serial_initialize(cli_serial_if_t* intf, const char* device, const SerialConfig* cfg)
{
  int   fd;

  fd = serial_init(device, cfg);
  if(fd < 0)
  {
    TRACE(CLI_SERIAL, "failed to initialize %s\n", device);
    return;
  }

  stream_init_with_fd(&intf->stream, fd, intf->rx_buffer, 128, 1024);

  intf->stream.cb             = handle_stream_event;
  intf->cli_intf.put_tx_data  = cli_put_tx_data;

  cli_intf_register(&intf->cli_intf);
  stream_start(&intf->stream);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
cli_serial_intf_init(void)
{
  const SerialConfig    cfg1 = 
  {
    .baud       = B115200,
    .data_bit   = 8,
    .stop_bit   = 1,
    .parity     = SerialParity_None,
  };

  TRACE(CLI_TELNET, "initializing cli serial interfaces\n");

  cli_serial_initialize(&_serial_if1, "/dev/ttyUSB5",  &cfg1);
}
