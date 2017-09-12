/*
 * modbus_rtu_snoop.c
 *
 * Created: 12/29/2016 11:59:01 AM
 *  Author: hkim
 */ 

/*
 * just a bunch of default empty user callback implementations
 */
#include <stdint.h>

#include "common.h"
#include "modbus_rtu_request_handler.h"
#include "modbus_funcs.h"

__attribute__((weak))
MBErrorCode
modbus_user_coil_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNCoils, MBRegisterMode eMode)
{
  return MB_EIO;
}

__attribute__((weak))
MBErrorCode
modbus_user_holding_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  return MB_EIO;
}

__attribute__((weak))
MBErrorCode
modbus_user_input_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs)
{
  return MB_EIO;
}

__attribute__((weak))
MBErrorCode
modbus_user_discrete_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNDiscrete)
{
  return MB_EIO;
}
