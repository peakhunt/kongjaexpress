#ifndef __TCP_CONNECTOR_DEF_H__
#define __TCP_CONNECTOR_DEF_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "task_timer.h"
#include "watcher.h"
#include "common.h"

typedef enum
{
  tcp_connector_success,
  tcp_connector_failure,
  tcp_connector_in_progress,
  tcp_connector_timeout,
} tcp_connector_status_t;

struct __tcp_connector;
typedef struct __tcp_connector tcp_connector_t;

typedef void (*tcp_connector_callback)(tcp_connector_t* conn, tcp_connector_status_t status);

struct __tcp_connector
{
  int                       sd;
  bool                      is_started;
  tcp_connector_callback    cb;

  task_timer_t              tmr;
  watcher_t                 watcher;
};

extern void tcp_connector_init(tcp_connector_t* conn, int sd);
extern void tcp_connector_deinit(tcp_connector_t* conn);
extern tcp_connector_status_t tcp_connector_connect(tcp_connector_t* conn,
    struct sockaddr* addr, socklen_t addrlen, double to);

extern tcp_connector_status_t tcp_connector_try_ipv4(tcp_connector_t* conn, struct sockaddr_in* addr, double to);
extern tcp_connector_status_t tcp_connectyr_try_unix_domain(tcp_connector_t* conn, struct sockaddr_un* addr, double to);

#endif /* !__TCP_CONNECTOR_DEF_H__ */
