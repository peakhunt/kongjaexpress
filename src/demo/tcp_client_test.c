#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"

typedef struct
{
  char*                 name;
  bool                  connected;
  tcp_connector_t         connector;
  task_timer_t          tmr;
  struct sockaddr_in    to;
} TcpClient;

static void tcp_client_task_start(task_t* task);
static void try_to_connect(TcpClient* cli);

static task_t   _tcp_client_task = 
{
  .name   = "TCP Client Task",
  .start  = tcp_client_task_start,
};


static TcpClient      _cli1,
                      _cli2;

static void
tcp_connector_event_handler(TcpClient* cli, tcp_connector_status_t status, bool direct)
{
  switch(status)
  {
  case tcp_connector_success:
    log_write("%s %s connected\n", __func__, direct ? "direct" : "indirect", cli->name);
    cli->connected    = true;
    task_timer_start(&cli->tmr, 5.0, 0.0);
    break;

  case tcp_connector_failure:
    log_write("%s %s failed\n", __func__, direct ? "directr" : "indirect", cli->name);

    tcp_connector_deinit(&cli->connector);

    task_timer_start(&cli->tmr, 3.0, 0.0);
    break;

  case tcp_connector_in_progress:
    /* impossible to occur */
    break;

  case tcp_connector_timeout:
    log_write("connect time out  %s\n", cli->name);

    tcp_connector_deinit(&cli->connector);

    try_to_connect(cli);
    break;
  }
}

static void
cli_connector_callback(tcp_connector_t* conn, tcp_connector_status_t status)
{
  TcpClient* cli = container_of(conn, TcpClient, connector);

  log_write("=============  %s, %s =================  \n", __func__, cli->name);

  tcp_connector_event_handler(cli, status, false);
}

static void
retry_timer_expired(task_timer_t* t, void* arg)
{
  TcpClient* cli = container_of(t, TcpClient, tmr);

  log_write("=============  %s, %s =================  \n", __func__, cli->name);
  
  if(cli->connected)
  {
    // in case this is connected, we setup a timer and
    // after the timer, we disconnect and try reconnect for test
    log_write("disconnecting %s\n", cli->name);

    tcp_connector_deinit(&cli->connector);
  }
  else
  {
    //
    // reconnect probation timeout expired
    //
    log_write("probation timed out  %s\n", cli->name);
  }

  try_to_connect(cli);
}

static void
try_to_connect(TcpClient* cli)
{
  tcp_connector_status_t ret;

  log_write("=============  %s, %s =================  \n", __func__, cli->name);

  cli->connected = false;

  ret = tcp_connector_try_ipv4(&cli->connector, &cli->to, 5.0);
  switch(ret)
  {
  case tcp_connector_success:
  case tcp_connector_failure:
    tcp_connector_event_handler(cli, ret, true);
    break;

  case tcp_connector_in_progress:
    log_write("%s connect in progress\n", cli->name);
    break;

  default:
    break;
  }
}

static void
tcp_client_task_start(task_t* task)
{
  log_write("%s tcp client task started\n", __func__);

  _cli1.name                = "Client1";
  _cli1.connected           = false;
  _cli1.to.sin_family       = AF_INET;
  _cli1.to.sin_addr.s_addr  = inet_addr("192.168.1.110");
  _cli1.to.sin_port         = htons(22);
  _cli1.connector.cb        = cli_connector_callback;
  task_timer_init(&_cli1.tmr, retry_timer_expired, NULL);

  _cli2.name                = "Client2";
  _cli2.connected           = false;
  _cli2.to.sin_family       = AF_INET;
  _cli2.to.sin_addr.s_addr  = inet_addr("8.8.8.8");
  _cli2.to.sin_port         = htons(9999);
  _cli2.connector.cb        = cli_connector_callback;
  task_timer_init(&_cli2.tmr, retry_timer_expired, NULL);

  try_to_connect(&_cli1);
  try_to_connect(&_cli2);
}

void
tcp_client_task_init(void)
{
  log_write("initializing tcp client task...\n");
  task_init(&_tcp_client_task);
}

void
tcp_client_task_run(void)
{
  log_write("running tcp client task...\n");
  task_run(&_tcp_client_task);
}
