#ifndef __TCP_SERVER_DEF_H__
#define __TCP_SERVER_DEF_H__

#include <ev.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "common.h"
#include "watcher.h"

#define MAX_ADDRESS_STRING_LEN            256

typedef enum
{
  tcp_server_type_ipv4,
  tcp_server_type_unix_domain,
} tcp_server_type_t;

struct __tcp_server;
typedef struct __tcp_server tcp_server_t;

typedef void (*tcp_server_rx_event)(tcp_server_t* server);
typedef void (*tcp_server_bound_addr)(tcp_server_t* server, char string[MAX_ADDRESS_STRING_LEN]);
typedef void (*tcp_new_connection_cb)(tcp_server_t* server, int newsd, struct sockaddr* from);

struct __tcp_server 
{
  int                       sd;
  bool                      is_listening;

  //
  // protocol specific callback
  //
  tcp_server_rx_event       rx_event;
  tcp_server_bound_addr     get_bound_addr;

  // 
  // user callback
  //
  tcp_new_connection_cb     conn_cb;

  union
  {
    struct sockaddr_in      ipv4_addr;
    struct sockaddr_un      unix_addr;
  };

  tcp_server_type_t         server_type;

  watcher_t                 watcher;
};

extern void tcp_server_init(tcp_server_t* server, int sd, tcp_server_rx_event cb);
extern void tcp_server_deinit(tcp_server_t* server);
extern void tcp_server_start(tcp_server_t* server);
extern void tcp_server_stop(tcp_server_t* server);
extern void tcp_server_get_port_name(tcp_server_t* server, char name[MAX_ADDRESS_STRING_LEN]);

#endif /* !__TCP_SERVER_DEF_H__ */
