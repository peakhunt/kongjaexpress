#include <unistd.h>
#include "common.h"
#include "trace.h"
#include "modbus_tcp_master.h"

#define MODBUS_TCP_MASTER_CONNECT_TIMEOUT       5.0
#define MODBUS_TCP_MASTER_PROBATION_TIMEOUT     1.0

static void modbus_tcp_master_connect(ModbusTCPMaster* master);

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
modbus_tcp_master_handle_tcp_connect_event(ModbusTCPMaster* master, tcp_connector_status_t status)
{
  int   sd;

  switch(status)
  {
  case tcp_connector_success:
    TRACE(MB_TCP_MASTER, "Connect success\n");

    master->tcp_state   = ModbusTCPMasterState_Connected;
    sd = tcp_connect_get_sd(&master->tcp_connector);
    tcp_connector_deinit(&master->tcp_connector);

    master->stream.cb    = modbus_tcp_master_stream_callback;
    stream_init_with_fd(&master->stream, sd, master->rx_bounce_buf,  128, 512);
    stream_start(&master->stream);
    break;

  case tcp_connector_failure:
    TRACE(MB_TCP_MASTER, "Connect failure\n");

    task_timer_start(&master->reconnect_tmr, MODBUS_TCP_MASTER_PROBATION_TIMEOUT, 0);

    master->tcp_state   = ModbusTCPMasterState_Not_Connected;
    tcp_connector_deinit(&master->tcp_connector);
    break;

  case tcp_connector_in_progress:
    TRACE(MB_TCP_MASTER, "Connect In Progress\n");
    master->tcp_state   = ModbusTCPMasterState_Connecting;
    break;

  case tcp_connector_timeout:
    TRACE(MB_TCP_MASTER, "Connect Timeout\n");

    master->tcp_state   = ModbusTCPMasterState_Not_Connected;
    tcp_connector_deinit(&master->tcp_connector);

    modbus_tcp_master_connect(master);
    break;
  }
}

static void
modbus_tcp_master_tcp_connector_callback(tcp_connector_t* con, tcp_connector_status_t status)
{
  ModbusTCPMaster* master = container_of(con, ModbusTCPMaster, tcp_connector);

  modbus_tcp_master_handle_tcp_connect_event(master, status);
}

static void
modbus_tcp_master_connect(ModbusTCPMaster* master)
{
  tcp_connector_status_t    ret;

  ret = tcp_connector_try_ipv4(&master->tcp_connector, &master->server_addr, MODBUS_TCP_MASTER_CONNECT_TIMEOUT);
  modbus_tcp_master_handle_tcp_connect_event(master, ret);
}

static void
modbus_tcp_master_reconnect_timeout(task_timer_t* te, void* unused)
{
  ModbusTCPMaster* master = container_of(te, ModbusTCPMaster, reconnect_tmr);

  TRACE(MB_TCP_MASTER, "ReConnect timer timeout\n");
  modbus_tcp_master_connect(master);
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

  task_timer_init(&master->reconnect_tmr, modbus_tcp_master_reconnect_timeout, NULL);
}

void
modbus_tcp_master_start(ModbusTCPMaster* master)
{
  modbus_tcp_master_connect(master);
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
    tcp_connector_deinit(&master->tcp_connector);
    break;

  case ModbusTCPMasterState_Connected:
    stream_deinit(&master->stream);
    break;
  }

  //
  // stop reconnect timer if it is active
  //
  if(task_timer_active(&master->reconnect_tmr))
  {
    task_timer_stop(&master->reconnect_tmr);
  }

  master->tcp_state   = ModbusTCPMasterState_Not_Connected;
}
