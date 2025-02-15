#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "esp_log.h"

#include "server.h"
#include "modbus.h"
#include "echo.h"

static const char *TAG = "server";

int SocketServer::Send(uint8_t* data, size_t len)
{
    size_t to_write = len;
    int written = -1 ;
    while (to_write > 0)
    {
        written = send(m_Sock, data + (len - to_write), to_write, 0);
        if (written < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
        to_write -= written;
    }
    return written;
}

void SocketServer::Listen()
{
    char addr_str[128];
    int addr_family = (int)AF_INET;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    
    uint16_t port;

    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(m_Port);
        port = ntohs(dest_addr_ip4->sin_port);
        ip_protocol = IPPROTO_IP;
    }

    m_ServerSock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (m_ServerSock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(m_ServerSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(m_ServerSock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(m_ServerSock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", port);

    err = listen(m_ServerSock, 1);
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Socket listening");
        socklen_t addr_len = sizeof(source_addr);
        m_Sock = accept(m_ServerSock, (struct sockaddr *)&source_addr, &addr_len);
        if (m_Sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(m_Sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(m_Sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(m_Sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(m_Sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET)
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        
        int len;
        uint8_t rx_buffer[128];
    
        do
        {
            len = recv(m_Sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0)
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            }
            else if (len == 0)
            {
                ESP_LOGW(TAG, "Connection closed");
            }
            else
            {
    
                ESP_LOGD(TAG, "Received %d bytes.", len);
                ProcessRx(rx_buffer, len);
            }
        } while (len > 0);
        
        
        shutdown(m_Sock, 0);
        close(m_Sock);
    }

CLEAN_UP:
    close(m_ServerSock);
    vTaskDelete(NULL);
}

void modbus_server_task(void *pvParameters)
{
    ModbusServer MyModbusServer(53, (int) pvParameters);
    MyModbusServer.Listen();

    vTaskDelete(NULL);
}

void echo_server_task(void *pvParameters)
{
    EchoServer MyEchoServer((int) pvParameters);
    MyEchoServer.Listen();

    vTaskDelete(NULL);
}
