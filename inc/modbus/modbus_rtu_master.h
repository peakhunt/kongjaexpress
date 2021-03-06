#ifndef __MODBUS_RTU_MASTER_DEF_H__
#define __MODBUS_RTU_MASTER_DEF_H__

#include <stdint.h>
#include "task_timer.h"
#include "watcher.h"
#include "stream.h"
#include "modbus_master.h"

typedef struct
{
  ModbusMasterCTX     ctx;
  uint8_t             rx_bounce_buf[128];

  stream_t            stream;

  task_timer_t        t35;
  double              t35_val;
} ModbusRTUMaster;

extern void modbus_rtu_master_init(ModbusRTUMaster* master, int fd);
extern void modbus_rtu_master_start(ModbusRTUMaster* master);
extern void modbus_rtu_master_stop(ModbusRTUMaster* master);

#endif /* !__MODBUS_RTU_MASTER_DEF_H__ */
