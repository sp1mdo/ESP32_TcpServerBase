#ifndef _ECHO_H
#define _ECHO_H

#include <inttypes.h>
#include <cstdlib>
#include <memory>

#include "server.h"

class BaseApplication;

class EchoApplication : public BaseApplication
{
public:
    EchoApplication() {};
    void ProcessRx(uint8_t *data, size_t len) override;
};

#endif