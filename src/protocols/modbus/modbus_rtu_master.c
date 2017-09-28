#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "modbus_master.h"
#include "modbus_rtu_master.h"
#include "modbus_crc.h"

#include "hex_dump.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_rtu_master_request(ModbusMasterCTX* ctx, uint8_t slave)
{
  ModbusRTUMaster*  master = container_of(ctx, ModbusRTUMaster, ctx);
  uint16_t    crc;

  ctx->tx_buf[0]    = slave;

  crc = modbus_calc_crc(ctx->tx_buf, ctx->tx_ndx);

  ctx->tx_buf[ctx->tx_ndx++] = (uint8_t)(crc & 0xff);
  ctx->tx_buf[ctx->tx_ndx++] = (uint8_t)(crc >> 8);

  TRACE_DUMP(MB_RTU_MASTER, "MB RTU Master TX", ctx->tx_buf, ctx->tx_ndx);

  if(stream_write(&master->stream, ctx->tx_buf, ctx->tx_ndx) == false)
  {
    TRACE(MB_RTU_MASTER, "tx error\n");
  }
}

static void
modbus_rtu_master_stream_callback(stream_t* stream, stream_event_t evt)
{
  ModbusRTUMaster*  master = container_of(stream, ModbusRTUMaster, stream);

  UNUSED(master);

  switch(evt)
  {
  case stream_event_rx:
    // XXX
    // FIXME RX handling
    //modbus_rtu_rx(slave);
    break;

  default:
    TRACE(MB_RTU_MASTER, "stream_event : %d\n", evt);
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_rtu_master_init(ModbusRTUMaster* master, int fd)
{
  master->ctx.pdu_offset    = 1;
  master->ctx.request       = modbus_rtu_master_request;
  mb_master_ctx_init(&master->ctx);

  master->stream.cb    = modbus_rtu_master_stream_callback;
  stream_init_with_fd(&master->stream, fd, master->rx_bounce_buf,  128, 512);
}

void
modbus_rtu_master_start(ModbusRTUMaster* master)
{
  stream_start(&master->stream);
}

void
modbus_rtu_master_stop(ModbusRTUMaster* master)
{
  stream_stop(&master->stream);
}
