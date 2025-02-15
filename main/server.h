#ifndef _SERVER_H
#define _SERVER_H

#define PORT 502
#define KEEPALIVE_IDLE 5
#define KEEPALIVE_INTERVAL 5
#define KEEPALIVE_COUNT 3

void modbus_server_task(void *pvParameters);
void echo_server_task(void *pvParameters);

class SocketServer;

class BaseApplication
{
public:
    virtual void ProcessRx(uint8_t *data, size_t len);
    SocketServer *m_Server;
};

class SocketServer
{
    BaseApplication &m_Application;

public:
    SocketServer(uint16_t port, BaseApplication &application) : m_Application(application), m_Port(port)
    {
        m_Application.m_Server = this;
    }
    ~SocketServer()
    {
        close(m_ServerSock);
    };

    void Listen();
    int Send(uint8_t *data, size_t len);

private:
    uint16_t m_Port;
    int m_Sock;
    int m_ServerSock;
};

#endif // _SERVER_H
