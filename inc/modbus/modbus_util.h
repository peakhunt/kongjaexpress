/*
 * modbus_util.h
 *
 * Created: 12/28/2016 3:39:25 PM
 *  Author: hkim
 */ 


#ifndef MODBUS_UTIL_H_
#define MODBUS_UTIL_H_

extern MBException prveMBError2Exception( MBErrorCode eErrorCode );
extern uint8_t xMBUtilGetBits( uint8_t * ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits );
extern void xMBUtilSetBits( uint8_t * ucByteBuf, uint16_t usBitOffset, uint8_t ucNBits, uint8_t ucValue );

#endif /* MODBUS_UTIL_H_ */