#include <unistd.h>
#include "common.h"
#include "trace.h"
#include "modbus_tcp_master.h"

#define MODBUS_TCP_MASTER_CONNECT_TIMEOUT       5.0
#define MODBUS_TCP_MASTER_PROBATION_TIMEOUT     1.0

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_request(ModbusMasterCTX* ctx, uint8_t slave)
{
  // FIXME XXX
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal tcp stream management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_stream_callback(stream_t* stream, stream_event_t evt)
{
  ModbusTCPMaster*  master = container_of(stream, ModbusTCPMaster, stream);

  UNUSED(master);

  switch(evt)
  {
  case stream_event_rx:
    // XXX
    // FIXME RX handling
    //modbus_rtu_rx(slave);
    break;

  default:
    TRACE(MB_TCP_MASTER, "stream_event : %d\n", evt);
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal tcp connection management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_tcp_connector_callback(tcp_auto_connector_t* con, int sd)
{
  ModbusTCPMaster* master = container_of(con, ModbusTCPMaster, tcp_connector);

  if(sd >= 0)
  {
    TRACE(MB_TCP_MASTER, "Connect success\n");

    master->tcp_state   = ModbusTCPMasterState_Connected;

    master->stream.cb    = modbus_tcp_master_stream_callback;
    stream_init_with_fd(&master->stream, sd, master->rx_bounce_buf,  128, 512);
    stream_start(&master->stream);
  }
  else
  {
    TRACE(MB_TCP_MASTER, "Connect failure\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_tcp_master_init(ModbusTCPMaster* master, struct sockaddr_in*  server_addr)
{
  master->ctx.request = modbus_tcp_master_request;
  
  mb_master_ctx_init(&master->ctx);

  memcpy(&master->server_addr, server_addr, sizeof(struct sockaddr_in));
  master->tcp_state   = ModbusTCPMasterState_Not_Connected;

  master->tcp_connector.cb = modbus_tcp_master_tcp_connector_callback;
  tcp_auto_connector_init(&master->tcp_connector, server_addr,
      MODBUS_TCP_MASTER_CONNECT_TIMEOUT,
      MODBUS_TCP_MASTER_PROBATION_TIMEOUT);
}

void
modbus_tcp_master_start(ModbusTCPMaster* master)
{
  tcp_auto_connector_start(&master->tcp_connector);
}

void
modbus_tcp_master_stop(ModbusTCPMaster* master)
{
  switch(master->tcp_state)
  {
  case ModbusTCPMasterState_Not_Connected:
    // nothing to do
    break;

  case ModbusTCPMasterState_Connecting:
    tcp_auto_connector_stop(&master->tcp_connector);
    break;

  case ModbusTCPMasterState_Connected:
    stream_deinit(&master->stream);
    break;
  }

  master->tcp_state   = ModbusTCPMasterState_Not_Connected;
}
