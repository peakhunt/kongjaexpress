#ifndef __MODBUS_MASTER_DEF_H__
#define __MODBUS_MASTER_DEF_H__

#include "modbus_common.h"

#define MODBUS_MASTER_MAX_BUFFER_SIZE           512

struct __modbus_master_ctx;
typedef struct __modbus_master_ctx ModbusMasterCTX;

struct __modbus_master_ctx
{
  uint8_t     tx_buf[MODBUS_MASTER_MAX_BUFFER_SIZE];
  uint16_t    tx_ndx;

  uint8_t     pdu_offset;

  // FIXME XXX
  // response callbacks

  // FIXME XXX
  // transport delegates
  void (*request)(ModbusMasterCTX* ctx, uint8_t slave);

  // FIXME XXX
};


void mb_master_ctx_init(ModbusMasterCTX* ctx);

void mb_master_read_coils_req(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb);
void mb_master_read_discrete_inputs(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb);
void mb_master_read_holding_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb);
void mb_master_read_input_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb);
void mb_master_write_single_coil(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t value);
void mb_master_write_single_register(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t value);
void mb_master_write_multiple_coils(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb,
    const uint8_t* values);
void mb_master_write_multiple_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb,
    const uint16_t* values);
void mb_master_write_and_read_registers(ModbusMasterCTX* ctx, uint8_t slave,
    uint16_t write_addr, uint16_t nb_write, const uint16_t* values,
    uint16_t read_addr, uint16_t nb_read);

void mb_master_handle_response(ModbusMasterCTX* ctx, uint8_t slave, uint8_t* pdu, int pdu_len);

#endif /* !__MODBUS_MASTER_DEF_H__ */
