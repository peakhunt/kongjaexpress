#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "modbus_master.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_master_fill_io_status_req(ModbusMasterCTX* ctx,
    uint8_t function, uint16_t reg_addr, uint8_t nb)
{
  // transport specific header will be filled in later
  ctx->tx_ndx   = ctx->pdu_offset;

  ctx->tx_buf[ctx->tx_ndx++]  = function;
  ctx->tx_buf[ctx->tx_ndx++]  = reg_addr >> 8;
  ctx->tx_buf[ctx->tx_ndx++]  = reg_addr & 0xff;
  ctx->tx_buf[ctx->tx_ndx++]  = nb >> 8;
  ctx->tx_buf[ctx->tx_ndx++]  = nb & 0xff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
mb_master_ctx_init(ModbusMasterCTX* ctx)
{
  // FIXME
}

void
mb_master_read_coils_req(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb)
{
  mb_master_fill_io_status_req(ctx, 0, reg_addr, nb);
}

void
mb_master_read_discrete_inputs(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb)
{
}

void
mb_master_read_holding_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb)
{
}

void
mb_master_read_input_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb)
{
}

void
mb_master_write_single_coil(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t value)
{
}

void
mb_master_write_single_register(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t value)
{
}

void
mb_master_write_multiple_coils(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb,
    const uint8_t* values)
{
}

void
mb_master_write_multiple_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t nb,
    const uint16_t* values)
{
}

void
mb_master_write_and_read_registers(ModbusMasterCTX* ctx, uint8_t slave,
    uint16_t write_addr, uint8_t nb_write, const uint16_t* values,
    uint16_t read_addr, uint8_t nb_read)
{
}
