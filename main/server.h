#ifndef _SERVER_H
#define _SERVER_H

#define PORT 502
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3


void tcp_server_task(void *pvParameters);
void do_retransmit(const int sock);

#endif // _SERVER_H
