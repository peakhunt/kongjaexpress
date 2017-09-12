#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kongja_express.h"

static void tcp_server_task_start(task_t* task);

static task_t     _tcp_server_task =
{
  .name   = "TCP Server Task",
  .start  = tcp_server_task_start,
};

static tcp_server_t     _ipv4_server;
static tcp_server_t     _unix_domain_server;
static task_timer_t     _test_timer;
static bool             _server_stopped = false;

static void
test_timer_expired(task_timer_t* t, void* arg)
{
  if(_server_stopped == false)
  {
    tcp_server_stop(&_ipv4_server);
    tcp_server_stop(&_unix_domain_server);
    log_write("stopped tcp servers\n");
  }
  else
  {
    tcp_server_start(&_ipv4_server);
    tcp_server_start(&_unix_domain_server);
    log_write("started tcp servers\n");
  }

  _server_stopped = !_server_stopped;
}

static void
tcp_server_new_client(tcp_server_t* server, int newsd, struct sockaddr* from)
{
  if(server == &_ipv4_server)
  {
    log_write("got new client connection from ipv4\n");
  }
  else if(server == &_unix_domain_server)
  {
    log_write("got new client connection from unix domain\n");
  }

  close(newsd);
}

static void
tcp_server_task_start(task_t* task)
{
  log_write("%s tcp server task started\n", __func__);

  tcp_server_ipv4_init(&_ipv4_server, 8590, 5);
  tcp_server_unix_domain_init(&_unix_domain_server, "/tmp/.kongja_express_path", 5);

  tcp_server_start(&_ipv4_server);
  tcp_server_start(&_unix_domain_server);

  _ipv4_server.conn_cb          = tcp_server_new_client;
  _unix_domain_server.conn_cb   = tcp_server_new_client;

  task_timer_init(&_test_timer, test_timer_expired, NULL);
  task_timer_start(&_test_timer, 5.0, 5.0);
}

void
tcp_server_task_init(void)
{
  log_write("initializing tcp server task...\n");
  task_init(&_tcp_server_task);
}

void
tcp_server_task_run(void)
{
  log_write("running tcp server task...\n");
  task_run(&_tcp_server_task);
}
