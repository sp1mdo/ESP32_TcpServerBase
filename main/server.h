#ifndef _SERVER_H
#define _SERVER_H

#include "lwip/sockets.h"

#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3

void modbus_server_task(void *pvParameters);
void echo_server_task(void *pvParameters);

class SocketServer
{
public:
    SocketServer(uint16_t port) : m_Port(port) { }
    ~SocketServer()
    {
        close(m_ServerSock);
    };

    void Listen();
    int Send(uint8_t *data, size_t len);
    virtual void ProcessRx(uint8_t *data, size_t len) = 0;

private:
    uint16_t m_Port;
    int m_Sock;
    int m_ServerSock;
    struct sockaddr_storage dest_addr;
    struct sockaddr_storage source_addr;
};

#endif // _SERVER_H
