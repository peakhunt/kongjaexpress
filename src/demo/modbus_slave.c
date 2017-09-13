#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"
#include "trace.h"
#include "serial.h"

#include "modbus_rtu.h"
#include "modbus_rtu_slave.h"
#include "modbus_tcp_slave.h"

static void modbus_task_start(task_t* task);

static task_t     _mb_task =
{
  .name   = "Modbus Task",
  .start  = modbus_task_start,
};

static ModbusRTUSlave   _rtu_slave;
static ModbusTCPSlave   _tcp_slave;

////////////////////////////////////////////////////////////////////////////////
//
// modbus slavef callbacks
//
////////////////////////////////////////////////////////////////////////////////
static MBErrorCode 
input_regs_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,uint16_t usAddress, uint16_t usNRegs)
{
  TRACE(MAIN, "read input regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  return MB_ENOERR;
}

static MBErrorCode
holding_regs_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  if(eMode == MB_REG_READ)
  {
    TRACE(MAIN, "read holding regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  }
  else
  {
    TRACE(MAIN, "write holding regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  }
  return MB_ENOERR;
}

static MBErrorCode
coil_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  if(eMode == MB_REG_READ)
  {
    TRACE(MAIN, "read coil regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  }
  else
  {
    TRACE(MAIN, "write coil regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  }
  return MB_ENOERR;
}

static MBErrorCode
discrete_cb(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs)
{
  TRACE(MAIN, "read discrete  regs req: addr: %d, reg addr: %d, num regs: %d\n", addr, usAddress, usNRegs);
  return MB_ENOERR;
}

////////////////////////////////////////////////////////////////////////////////
//
// start routine
//
////////////////////////////////////////////////////////////////////////////////
static void
modbus_task_start(task_t* task)
{
  const SerialConfig    cfg1 = 
  {
    .baud       = B9600,
    .data_bit   = 8,
    .stop_bit   = 1,
    .parity     = SerialParity_None,
  };
  int   fd;

  TRACE(MAIN, "%s modbus task started\n", __func__);

  fd = serial_init("/dev/ttyUSB0", &cfg1);
  if(fd < 0)
  {
    TRACE(MAIN, "failed to initialize serial device for RTU slave\n");
  }

  _rtu_slave.ctx.input_regs_cb    = input_regs_cb;
  _rtu_slave.ctx.holding_regs_cb  = holding_regs_cb;
  _rtu_slave.ctx.coil_cb          = coil_cb;
  _rtu_slave.ctx.discrete_cb      = discrete_cb;

  _tcp_slave.ctx.input_regs_cb    = input_regs_cb;
  _tcp_slave.ctx.holding_regs_cb  = holding_regs_cb;
  _tcp_slave.ctx.coil_cb          = coil_cb;
  _tcp_slave.ctx.discrete_cb      = discrete_cb;



  if(fd > 0)
  {
    modbus_rtu_slave_init(&_rtu_slave, 1, fd);
    modbus_rtu_slave_start(&_rtu_slave);
  }

  modbus_tcp_slave_init(&_tcp_slave, 2, 12345);
  modbus_tcp_slave_start(&_tcp_slave);

}

////////////////////////////////////////////////////////////////////////////////
//
// startup
//
////////////////////////////////////////////////////////////////////////////////
void
modbus_slave_task_init(void)
{
  TRACE(MAIN, "initializing modbus task...\n");
  task_init(&_mb_task);
}

void
modbus_slave_task_run(void)
{
  TRACE(MAIN, "running  modbus task...\n");
  task_run(&_mb_task);
}
