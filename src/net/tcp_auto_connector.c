#include "trace.h"
#include "tcp_auto_connector.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  private utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
tcp_auto_connector_handle_tcp_connect_event(tcp_auto_connector_t* conn, tcp_connector_status_t status)
{
  switch(status)
  {
  case tcp_connector_success:
    //
    // invoke callback
    //
    conn->state = tcp_auto_connector_state_not_started;
    break;

  case tcp_connector_failure:
    conn->state = tcp_auto_connector_state_reconnect_wait;
    task_timer_start(&conn->reconn_wait_tmr, conn->reconn_wait_tmr_value, 0);
    break;

  case tcp_connector_in_progress:
    //
    // nothing todo 
    //
    conn->state = tcp_auto_connector_state_connecting;
    break;

  case tcp_connector_timeout:
    //
    // try reconnect
    //
    conn->state = tcp_auto_connector_state_connecting;
    break;
  }
}

static void
tcp_auto_connector_connect(tcp_auto_connector_t* conn)
{
  tcp_connector_status_t    ret;

  ret = tcp_connector_try_ipv4(&conn->tcp_connector, &conn->server_addr, conn->conn_tmr_value);
  tcp_auto_connector_handle_tcp_connect_event(conn, ret);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// callbacks
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
tcp_auto_connector_reconn_wait_timeout(task_timer_t* te, void* unused)
{
  tcp_auto_connector_t* conn = container_of(te, tcp_auto_connector_t, reconn_wait_tmr);

  tcp_auto_connector_connect(conn);
}

static void
tcp_auto_connector_tcp_connector_callback(tcp_connector_t* tconn,
    tcp_connector_status_t status)
{
  tcp_auto_connector_t* conn = container_of(tconn, tcp_auto_connector_t, tcp_connector);

  tcp_auto_connector_handle_tcp_connect_event(conn, status);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
tcp_auto_connector_init(tcp_auto_connector_t* conn,
    struct sockaddr_in* server_addr, double conn_tmr, double reconn_wait_tmr)
{
  memcpy(&conn->server_addr, server_addr, sizeof(struct sockaddr_in));

  conn->conn_tmr_value        = conn_tmr;
  conn->reconn_wait_tmr_value = reconn_wait_tmr;

  conn->tcp_connector.cb    = tcp_auto_connector_tcp_connector_callback;

  task_timer_init(&conn->reconn_wait_tmr, tcp_auto_connector_reconn_wait_timeout, NULL);

  conn->state = tcp_auto_connector_state_not_started;
}

void
tcp_auto_connector_start(tcp_auto_connector_t* conn)
{
  tcp_auto_connector_connect(conn);
}

void
tcp_auto_connecotr_stop(tcp_auto_connector_t* conn)
{
}
