#include <stdint.h>
#include "common.h"
#include "modbus_rtu_response_handler.h"
#include "modbus_funcs.h"

void
modbus_rtu_handler_response_rx(ModbusMasterCTX* ctx, uint8_t slave, uint8_t* pdu, int pdu_len)
{
  // FIXME
}
