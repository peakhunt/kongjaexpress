﻿/*
 * modbus_rtu.h
 *
 * Created: 12/28/2016 9:54:56 AM
 *  Author: hkim
 */ 


#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_

#include <stdint.h>

#define MB_RTU_MAX_PDU_SIZE             512     // just big enough

#define MB_PDU_SIZE_MAX                 253
#define MB_PDU_SIZE_MIN                 1
#define MB_PDU_FUNC_OFF                 0
#define MB_PDU_DATA_OFF                 1

#define MB_ADDRESS_BROADCAST            0
#define MB_ADDRESS_MIN                  1
#define MB_ADDRESS_MAX                  247

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// a bunch of definitions for standard MODBUS RTU
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MB_FUNC_NONE                          (  0 )
#define MB_FUNC_READ_COILS                    (  1 )
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
#define MB_FUNC_READ_HOLDING_REGISTER         (  3 )
#define MB_FUNC_READ_INPUT_REGISTER           (  4 )
#define MB_FUNC_WRITE_REGISTER                (  6 )
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS  ( 23 )
#define MB_FUNC_DIAG_READ_EXCEPTION           (  7 )
#define MB_FUNC_DIAG_DIAGNOSTIC               (  8 )
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT        ( 11 )
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG        ( 12 )
#define MB_FUNC_OTHER_REPORT_SLAVEID          ( 17 )
#define MB_FUNC_ERROR                         ( 128)
 
typedef enum
{
  MB_EX_NONE                    = 0x00,
  MB_EX_ILLEGAL_FUNCTION        = 0x01,
  MB_EX_ILLEGAL_DATA_ADDRESS    = 0x02,
  MB_EX_ILLEGAL_DATA_VALUE      = 0x03,
  MB_EX_SLAVE_DEVICE_FAILURE    = 0x04,
  MB_EX_ACKNOWLEDGE             = 0x05,
  MB_EX_SLAVE_BUSY              = 0x06,
  MB_EX_MEMORY_PARITY_ERROR     = 0x08,
  MB_EX_GATEWAY_PATH_FAILED     = 0x0A,
  MB_EX_GATEWAY_TGT_FAILED      = 0x0B
} MBException;

typedef enum
{
  MB_ENOERR,                  /*!< no error. */
  MB_ENOREG,                  /*!< illegal register address. */
  MB_EINVAL,                  /*!< illegal argument. */
  MB_EPORTERR,                /*!< porting layer error. */
  MB_ENORES,                  /*!< insufficient resources. */
  MB_EIO,                     /*!< I/O error. */
  MB_EILLSTATE,               /*!< protocol stack in illegal state. */
  MB_ETIMEDOUT                /*!< timeout error occurred. */
} MBErrorCode;

typedef enum
{
  MB_REG_READ,                /*!< Read register values and pass to protocol stack. */
  MB_REG_WRITE                /*!< Update register values. */
} MBRegisterMode;

struct __modbus_ctx;
typedef struct __modbus_ctx ModbusCTX;

struct __modbus_ctx
{
  uint8_t               data_buffer[MB_RTU_MAX_PDU_SIZE];
  uint16_t              data_ndx;
  uint16_t              tx_ndx;

  uint32_t              rx_buffer_overflow;
  uint32_t              rx_crc_error;
  uint32_t              rx_frames;
  uint32_t              req_fails;
  uint32_t              my_frames;
  uint32_t              tx_frames;
  
  MBErrorCode           (*input_regs_cb)(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                            uint16_t usAddress, uint16_t usNRegs);
  MBErrorCode           (*holding_regs_cb)(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                            uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode);
  MBErrorCode           (*coil_cb)(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
                            uint16_t usNRegs, MBRegisterMode eMode);
  MBErrorCode           (*discrete_cb)(ModbusCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
                            uint16_t usNRegs);
};

static inline void
mb_ctx_reset_data_buffer(ModbusCTX* ctx)
{
  ctx->data_ndx   = 0;
  ctx->tx_ndx     = 0;
}

static inline void
mb_ctx_put_data(ModbusCTX* ctx, uint8_t b)
{
  ctx->data_buffer[ctx->data_ndx] = b;
  ctx->data_ndx++;
}

static inline uint16_t
mb_ctx_rx_len(ModbusCTX* ctx)
{
  return ctx->data_ndx;
}

static inline uint8_t*
mb_ctx_buffer(ModbusCTX* ctx)
{
  return ctx->data_buffer;
}

static inline void
mb_ctx_init(ModbusCTX* ctx)
{
  ctx->rx_buffer_overflow       = 0;
  ctx->rx_crc_error             = 0;
  ctx->rx_frames                = 0;
  ctx->req_fails                = 0;
  ctx->my_frames                = 0;
  ctx->tx_frames                = 0;
}

#endif /* MODBUS_RTU_H_ */
