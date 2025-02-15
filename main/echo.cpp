#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "esp_log.h"

#include "echo.h"


static const char *TAG = "echo";

void EchoServer::ProcessRx(uint8_t *data, size_t len)
{
    
    ESP_LOGI(TAG, "%c", data[0]);
    printf("%c",data[0]);
    Send(data, len);
}

