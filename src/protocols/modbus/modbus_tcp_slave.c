#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "modbus_rtu.h"
#include "modbus_tcp_slave.h"
#include "modbus_crc.h"
#include "modbus_rtu_request_handler.h"
#include "modbus_funcs.h"
#include "mbap_reader.h"

#include "stream.h"
#include "sock_util.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head      le;
  stream_t              stream;
  ModbusTCPSlave*       slave;
  uint8_t               rx_buf[512];

  struct sockaddr_in    from;

  mbap_reader_t         mbap_reader;
} modbus_tcp_slave_connection_t;

static void modbus_tcp_slave_handle_stream_event(stream_t* stream, stream_event_t evt);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// connection alloc/dealloc
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_got_frame(mbap_reader_t* mbap)
{
  //
  // FIXME RX handler
  //
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// connection alloc/dealloc
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_alloc_conn(ModbusTCPSlave* slave, int newsd, struct sockaddr_in* from)
{
  modbus_tcp_slave_connection_t*    conn;

  TRACE(MB_TCP_SLAVE, "got new connection\n");

  conn = malloc(sizeof(modbus_tcp_slave_connection_t));
  if(conn == NULL)
  {
    TRACE(MB_TCP_SLAVE, "connection alloc failed\n");
    close(newsd);
    return;
  }

  INIT_LIST_HEAD(&conn->le);

  memcpy(&conn->from, from, sizeof(struct sockaddr_in));

  stream_init_with_fd(&conn->stream, newsd, conn->rx_buf, 512, 512);
  sock_util_put_nonblock(newsd);

  list_add_tail(&conn->le,  &slave->connections);

  mbap_reader_init(&conn->mbap_reader, modbus_tcp_slave_got_frame);

  conn->stream.cb   = modbus_tcp_slave_handle_stream_event;
  stream_start(&conn->stream);
}

static void
modbus_tcp_slave_dealloc_conn(modbus_tcp_slave_connection_t* conn)
{
  stream_deinit(&conn->stream);
  list_del_init(&conn->le);
  free(conn);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stream callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_handle_stream_event(stream_t* stream, stream_event_t evt)
{
  modbus_tcp_slave_connection_t* conn = container_of(stream, modbus_tcp_slave_connection_t, stream);

  switch(evt)
  {
  case stream_event_rx:
    mbap_reader_feed(&conn->mbap_reader, stream->rx_buf, stream->rx_data_len);
    break;

  case stream_event_eof:
    TRACE(MB_TCP_SLAVE, "connection eof detected\n");
    modbus_tcp_slave_dealloc_conn(conn);
    break;

  case stream_event_err:
    TRACE(MB_TCP_SLAVE, "connection error detected\n");
    modbus_tcp_slave_dealloc_conn(conn);
    break;

  case stream_event_tx:
    // never occurs
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// new connection callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_new_connection(tcp_server_t* server, int newsd, struct sockaddr* from)
{
  ModbusTCPSlave* slave = container_of(server, ModbusTCPSlave, server);

  modbus_tcp_slave_alloc_conn(slave, newsd, (struct sockaddr_in*)from);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_tcp_slave_init(ModbusTCPSlave* slave, uint8_t device_addr, int tcp_port)
{
  mb_ctx_init(&slave->ctx);

  slave->my_address = device_addr;

  slave->server.conn_cb = modbus_tcp_slave_new_connection;
  tcp_server_ipv4_init(&slave->server, tcp_port, 5);

  INIT_LIST_HEAD(&slave->connections);
}

void
modbus_tcp_slave_start(ModbusTCPSlave* slave)
{
  tcp_server_start(&slave->server);
}

void
modbus_tcp_slave_stop(ModbusTCPSlave* slave)
{
  modbus_tcp_slave_connection_t   *p, *n;

  tcp_server_stop(&slave->server);

  list_for_each_entry_safe(p, n, &slave->connections, le)
  {
    modbus_tcp_slave_dealloc_conn(p);
  }
}
