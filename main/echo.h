#ifndef _ECHO_H
#define _ECHO_H

#include <inttypes.h>
#include <cstdlib>
#include <memory>

#include "server.h"

class EchoServer : public SocketServer
{
public:
    EchoServer(uint16_t port) : SocketServer(port)
    { }
    void ProcessRx(uint8_t *data, size_t len) override;
};

#endif