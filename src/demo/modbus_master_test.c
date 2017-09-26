#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"
#include "trace.h"
#include "serial.h"

#include "modbus_master.h"
#include "modbus_rtu_master.h"
#include "modbus_tcp_master.h"

static void modbus_master_task_start(task_t* task);

static task_t     _mb_master_task =
{
  .name   = "Modbus Master Task",
  .start  = modbus_master_task_start,
};

static ModbusTCPMaster   _tcp_master;

static void
modbus_master_task_start(task_t* task)
{
  struct sockaddr_in  server_addr;

  TRACE(MAIN, "%s modbus master task started\n", __func__);

  server_addr.sin_family       = AF_INET;
  server_addr.sin_addr.s_addr  = inet_addr("192.168.1.110");
  server_addr.sin_port         = htons(8908);

  modbus_tcp_master_init(&_tcp_master,  &server_addr);
  modbus_tcp_master_start(&_tcp_master);
}

////////////////////////////////////////////////////////////////////////////////
//
// startup
//
////////////////////////////////////////////////////////////////////////////////
void
modbus_master_task_init(void)
{
  TRACE(MAIN, "initializing modbus master task...\n");
  task_init(&_mb_master_task);
}

void
modbus_master_task_run(void)
{
  TRACE(MAIN, "running  modbus master task...\n");
  task_run(&_mb_master_task);
}
