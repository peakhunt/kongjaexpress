/*
 * modbus_rtu_request_handler.h
 *
 * Created: 12/28/2016 2:30:38 PM
 *  Author: hkim
 */ 


#ifndef MODBUS_RTU_REQUEST_HANDLER_H_
#define MODBUS_RTU_REQUEST_HANDLER_H_

#include "modbus_rtu.h"

extern uint8_t modbus_rtu_handler_request_rx(ModbusCTX* ctx, uint8_t addr, uint16_t len, uint8_t* pdu, uint16_t* rsp_len);

#endif /* MODBUS_RTU_REQUEST_HANDLER_H_ */
