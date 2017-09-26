#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"
#include "trace.h"

extern void tcp_server_task_init(void);
extern void tcp_server_task_run(void);

extern void tcp_client_task_init(void);
extern void tcp_client_task_run(void);

extern void cli_task_init(void);
extern void cli_task_run(void);

extern void modbus_slave_task_init(void);
extern void modbus_slave_task_run(void);

extern void modbus_master_task_init(void);
extern void modbus_master_task_run(void);

static uint32_t   _trace_init[] = 
{
  TRACE_COMP(MAIN),
  TRACE_COMP(TEST),
  TRACE_COMP(TASK),
  TRACE_COMP(SOCK_ERR),
  TRACE_COMP(SERIAL),
  TRACE_COMP(CLI_TELNET),
  TRACE_COMP(CLI_SERIAL),
  TRACE_COMP(CLI),
  TRACE_COMP(MB_TCP_SLAVE),
  TRACE_COMP(MBAP),
  TRACE_COMP(MB_RTU_SLAVE),
  TRACE_COMP(MB_RTU_MASTER),
  TRACE_COMP(MB_TCP_MASTER),
};

int
main()
{
  kongja_express_init();
  trace_init(_trace_init, sizeof(_trace_init)/sizeof(uint32_t));

  /*
  tcp_server_task_init();
  tcp_client_task_init();

  tcp_server_task_run();
  tcp_client_task_run();
  */
  cli_task_init();
  cli_task_run();

  modbus_slave_task_init();
  modbus_slave_task_run();

  modbus_master_task_init();
  modbus_master_task_run();

  idle_task_run();
}
