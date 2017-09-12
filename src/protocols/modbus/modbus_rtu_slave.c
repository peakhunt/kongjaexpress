﻿/*
 * modbus_rtu.c
 *
 * Created: 12/28/2016 9:55:53 AM
 *  Author: hkim
 */ 
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "modbus_rtu.h"
#include "modbus_rtu_slave.h"
#include "modbus_crc.h"
#include "modbus_rtu_request_handler.h"
#include "modbus_funcs.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// macros for convenience
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define INC_ERR_CNT(err_cnt)    \
  if(err_cnt < 0xffff)          \
  {                             \
    err_cnt++;                  \
  }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private prototoypes
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void t35_timeout_handler(task_timer_t* te, void* unused);
static void mb_rtu_start_handling_rx_frame(ModbusRTUSlave* slave);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// USART/RS486 specific
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_rtu_enable_rx(ModbusRTUSlave* slave)
{
  mb_ctx_reset_data_buffer(&slave->ctx);

  // 
  // FIXME enable RX in case of half-duplex
  //
}

static inline void
mb_rtu_disable_rx(ModbusRTUSlave* slave)
{
  //
  // FIXME disable RX inc ase of half-duplex
  //
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TX routine
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_rtu_start_tx(ModbusRTUSlave* slave)
{
  int   nwritten = 0,
        ret;

  ModbusCTX*    ctx = &slave->ctx;

  while(nwritten < ctx->data_ndx)
  {
    ret = write(slave->fd, &ctx->data_buffer[nwritten], ctx->data_ndx - nwritten);
    if(ret < 0 && !(errno == EWOULDBLOCK || errno == EAGAIN))
    {
      // FIXME TX error 
      break;
    }

    if(nwritten >= ctx->data_ndx)
    {
      ctx->tx_frames++;
    }
  }

  mb_rtu_enable_rx(slave);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// optional snooping
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__attribute__((weak))
void
modbus_rtu_frame_snoop(ModbusRTUSlave* slave,uint8_t addr, uint8_t* pdu, uint16_t len)
{
  // do nothing by default
}

__attribute__((weak))
void
modbus_rtu_init_snoop(ModbusRTUSlave* slave)
{
  // do nothing by default
}  


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MODBUS RX/TX in mainloop
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
t35_timeout_handler(task_timer_t* te, void* unused)
{
  ModbusRTUSlave*    slave = container_of(te, ModbusRTUSlave, t35);

  // for safely and to prevent fuck up, disable further rx here
  mb_rtu_disable_rx(slave);
  mb_rtu_start_handling_rx_frame(slave);
}

static void
mb_rtu_handle_rx_frame(ModbusRTUSlave* slave)
{
  uint8_t   addr,
  *pdu,
  ret;
  uint16_t  len;
  ModbusCTX*    ctx = &slave->ctx;

  addr = mb_ctx_buffer(ctx)[MB_SER_PDU_ADDR_OFF];
  len  = mb_ctx_rx_len(ctx) - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC;
  pdu  = (uint8_t*)&mb_ctx_buffer(ctx)[MB_SER_PDU_PDU_OFF];

  if(addr == slave->my_address || addr == MB_ADDRESS_BROADCAST)
  {
    ret = modbus_rtu_handler_request_rx(ctx, addr, len, pdu);
    if(addr != MB_ADDRESS_BROADCAST)
    {
      ctx->my_frames++;
      if(ret != true)
      {
        ctx->req_fails++;
      }
      mb_rtu_start_tx(slave);
      return;
    }
  }
  else
  {
    modbus_rtu_frame_snoop(slave, addr, pdu, len);
  }
  mb_rtu_enable_rx(slave);
}

static void
mb_rtu_start_handling_rx_frame(ModbusRTUSlave* slave)
{
  ModbusCTX*    ctx = &slave->ctx;

  ctx->rx_frames++;

  if(mb_ctx_rx_len(ctx) == 0)
  {
    // buffer overflow due to long frame has just occurred.
    mb_rtu_enable_rx(slave);
    return;
  }

  if(mb_ctx_rx_len(ctx) < MB_SER_RTU_PDU_SIZE_MIN)
  {
    INC_ERR_CNT(slave->rx_short_frame);
    mb_rtu_enable_rx(slave);
    return;
  }

  if(modbus_calc_crc((uint8_t*)mb_ctx_buffer(ctx), mb_ctx_rx_len(ctx)) != 0)
  {
    // crc16 error
    INC_ERR_CNT(ctx->rx_crc_error);
    mb_rtu_enable_rx(slave);
    return;
  }

  mb_rtu_handle_rx_frame(slave);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MODBUS utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
init_modbus_context(ModbusRTUSlave* slave, uint8_t addr)
{
  ModbusCTX*    ctx = &slave->ctx;

  mb_ctx_init(ctx);

  slave->my_address   = addr;
  slave->extension    = NULL;

  mb_ctx_reset_data_buffer(ctx);

  task_timer_init(&slave->t35, t35_timeout_handler, NULL);

  // T35
  // at 9600    : 3ms +- alpha
  // above 9600 : 2ms should be enough
  //
  // so we can safely go with 3ms for t35.
  //
  slave->t35_val                  = 8;        // just 8ms max for every baud rate. 3ms buffer.

  slave->rx_usart_overflow        = 0;
  slave->rx_usart_frame_error     = 0;
  slave->rx_usart_parity_error    = 0;
  slave->rx_short_frame           = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// watcher callbacks
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_rtu_rx(ModbusRTUSlave* slave)
{
  uint8_t       buffer[128];
  int           i,
                ret;
  ModbusCTX*    ctx = &slave->ctx;

  ret = read(slave->fd, buffer, 128);
  if(ret <= 0)
  {
    return;
  }

  for(i = 0; i < ret; i++)
  {
    if(ctx->data_ndx >= MB_SER_RTU_PDU_SIZE_MAX)
    {
      mb_ctx_reset_data_buffer(ctx);
      INC_ERR_CNT(ctx->rx_buffer_overflow);
      return;
    }
    mb_ctx_put_data(ctx, buffer[i]);
  }

  task_timer_restart(&slave->t35, slave->t35_val, 0);
}

static void
modbus_rtu_watcher_callback(watcher_t* watcher, watcher_event_t evt)
{
  ModbusRTUSlave*    slave = container_of(watcher, ModbusRTUSlave, watcher);

  switch(evt)
  {
  case watcher_event_rx:
    modbus_rtu_rx(slave);
    break;

  default:
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_rtu_init(ModbusRTUSlave* slave, uint8_t device_addr, int fd)
{
  slave->fd   = fd;

  watcher_init_with_fd(&slave->watcher, fd, modbus_rtu_watcher_callback);
  watcher_watch_rx(&slave->watcher);

  init_modbus_context(slave, device_addr);
  modbus_rtu_init_snoop(slave);
  mb_rtu_enable_rx(slave);
}

void
modbus_rtu_start(ModbusRTUSlave* slave)
{
  watcher_start(&slave->watcher);
}

void
modbus_rtu_stop(ModbusRTUSlave* slave)
{
  watcher_stop(&slave->watcher);
}
