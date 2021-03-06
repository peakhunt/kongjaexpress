#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/telnet.h>

#include "common.h"
#include "cli.h"
#include "tcp_server.h"
#include "tcp_server_ipv4.h"
#include "stream.h"
#include "sock_util.h"
#include "log.h"
#include "telnet_reader.h"
#include "trace.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head          le;           // a list of connections in cli_server
  stream_t                  stream;       // connected stream
  cli_intf_t                cli_intf;     // cli interface
  telnet_reader_t           telnet;     
  uint8_t                   rx_buf[128];
} cli_connection_t;

typedef struct
{
  tcp_server_t        tcp_server;
  struct list_head    conns;
} cli_server_t;

static void handle_stream_event(stream_t* stream, stream_event_t evt);
static void cli_put_tx_data(cli_intf_t* intf, char* data, int len);
static void telnet_rx_databack(telnet_reader_t* tr, uint8_t data);
static void telnet_rx_cmdback(telnet_reader_t* tr);

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
static cli_server_t     _cli_ipv4_server;

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
static void
init_telnet_session(cli_connection_t* conn)
{
  static const char iacs_to_send[] =
  {
    IAC, WILL,   TELOPT_SGA,         
    IAC, WILL,   TELOPT_ECHO,
  };

  stream_write(&conn->stream, (uint8_t*)iacs_to_send, sizeof(iacs_to_send));
}

static void
alloc_cli_connection(cli_server_t* server, int newsd)
{
  cli_connection_t*   conn;
  uint8_t             dummy = '\r';

  conn = malloc(sizeof(cli_connection_t));
  if(conn == NULL)
  {
    close(newsd);
    return;
  }

  INIT_LIST_HEAD(&conn->le);
  list_add_tail(&conn->le,  &server->conns);

  stream_init_with_fd(&conn->stream, newsd, conn->rx_buf, 128, 128);

  sock_util_put_nonblock(newsd);

  //
  // setup callbacks
  //
  conn->stream.cb   = handle_stream_event;

  //
  // register CLI interface
  //
  conn->cli_intf.put_tx_data = cli_put_tx_data;

  cli_intf_register(&conn->cli_intf);

  stream_start(&conn->stream);

  conn->telnet.databack = telnet_rx_databack;
  conn->telnet.cmdback  = telnet_rx_cmdback;
  telnet_reader_init(&conn->telnet);

  init_telnet_session(conn);

  // show prompt
  cli_handle_rx(&conn->cli_intf, &dummy, 1);
}

static void
dealloc_cli_connection(cli_connection_t* conn)
{
  stream_deinit(&conn->stream);

  cli_intf_unregister(&conn->cli_intf);

  list_del_init(&conn->le);

  free(conn);
}

////////////////////////////////////////////////////////////////////////////////
//
// telnet callbacks
//
////////////////////////////////////////////////////////////////////////////////
static void
telnet_rx_databack(telnet_reader_t* tr, uint8_t data)
{
  cli_connection_t* conn = container_of(tr, cli_connection_t, telnet);

  if(data == 0)
  {
    return;
  }
  cli_handle_rx(&conn->cli_intf, &data, 1);
}

static void
telnet_rx_cmdback(telnet_reader_t* tr)
{
  // ignore
}

////////////////////////////////////////////////////////////////////////////////
//
// cli interfaces
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_put_tx_data(cli_intf_t* intf, char* data, int len)
{
  cli_connection_t* conn = container_of(intf, cli_connection_t, cli_intf);

  stream_write(&conn->stream, (uint8_t*)data, len);
}

////////////////////////////////////////////////////////////////////////////////
//
// stream callback
//
////////////////////////////////////////////////////////////////////////////////
static void
handle_stream_event(stream_t* stream, stream_event_t evt)
{
  cli_connection_t* conn = container_of(stream, cli_connection_t, stream);

  //log_write("handle_stream_event %d\n", evt);
  switch(evt)
  {
  case stream_event_rx:
    telnet_reader_feed(&conn->telnet, stream->rx_buf, stream->rx_data_len);
    break;

  case stream_event_eof:
    TRACE(CLI_TELNET, "connection eof detected\n");
    dealloc_cli_connection(conn);
    break;

  case stream_event_err:
    TRACE(CLI_TELNET, "connection error detected\n");
    dealloc_cli_connection(conn);
    break;

  case stream_event_tx:
    // never occurs
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// tcp server callback
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_server_new_connection(tcp_server_t* server, int newsd, struct sockaddr* from)
{
  cli_server_t* cli_server = container_of(server, cli_server_t, tcp_server);
  TRACE(CLI_TELNET, "new cli connection\n");
  alloc_cli_connection(cli_server, newsd);
}

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
static
void cli_telnet_server_init(cli_server_t* server,  int port, int backlog)
{
  INIT_LIST_HEAD(&server->conns);

  server->tcp_server.conn_cb    = cli_server_new_connection;
  tcp_server_ipv4_init(&server->tcp_server, port, backlog);
  tcp_server_start(&server->tcp_server);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
cli_telnet_intf_init(void)
{
  TRACE(CLI_TELNET, "initializing cli telnet interface\n");

  cli_telnet_server_init(&_cli_ipv4_server, 9090, 5);
}
