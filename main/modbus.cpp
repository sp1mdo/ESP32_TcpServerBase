#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "esp_log.h"

#include "modbus.h"

static const char *TAG = "modbus";

uint16_t holdingRegisters[100] = {0};
uint16_t inputRegisters[100] = {0};
bool coils[8] = {true, false, false, false, false, false, false, true};

std::unique_ptr<modbus_query_t> parse_modbus_tcp_raw_data(const uint8_t *data, size_t len)
{
    //modbus_query_t *query = (modbus_query_t *)malloc(sizeof(modbus_query_t));
    auto query = std::unique_ptr<modbus_query_t>(new modbus_query_t); // Safer allocation
    query->transaction_id = data[1] + (data[0] << 8);
    query->protocol_id = ntohs(*(uint16_t *)&data[2]);
    query->length = ntohs(*(uint16_t *)&data[4]);
    query->unit_id = data[6];
    query->function_code = data[7];
    query->start_addr = ntohs(*(uint16_t *)&data[8]);
    query->word_count = ntohs(*(uint16_t *)&data[10]);

    return query;
}

int tcp_server_recv(uint8_t *payload, size_t length, const int sock)
{
    std::unique_ptr<modbus_query_t> query = parse_modbus_tcp_raw_data((uint8_t *)payload, length);
    if (query == NULL)
    {
        ESP_LOGE(TAG, "query is NULL.\n");
        return -1;
    }

    if (query->unit_id != 53)
    {
        ESP_LOGE(TAG, "other ID\n");
        return -1;
    }

    uint16_t *registers = (uint16_t *)payload + 10;
    //uint8_t *send_buf = (uint8_t *)malloc(12 + 2 * (query->word_count));
    uint8_t *send_buf = new uint8_t[12 + sizeof(uint16_t) * (query->word_count)];
    if (send_buf == NULL)
    {
        ESP_LOGE(TAG, "send_buf is NULL.\n");
        return -1;
    }

    modbus_reply_t *reply = (modbus_reply_t *)send_buf;

    reply->transaction_id = htons(query->transaction_id);
    reply->protocol_id = htons(query->protocol_id);
    reply->length = htons(3 + sizeof(uint16_t) * (query->word_count));
    reply->unit_id = query->unit_id;
    reply->function_code = query->function_code;
    reply->byte_count = (uint8_t)(sizeof(uint16_t) * query->word_count);

    uint8_t tmp_bytes = 0;
    uint16_t *offset = (uint16_t *)(send_buf + sizeof(modbus_reply_t));

    switch (query->function_code)
    {
    case READ_COILS:
    {
        // printf("Read Coils\n");
        reply->byte_count = (uint8_t)(query->word_count / 8);
        uint8_t *offset8 = (uint8_t *)offset;
        memset(offset, 0, reply->byte_count);
        for (size_t i = 0; i < query->word_count; i++)
        {
            if (coils[i] == true)
                offset8[i / 8] |= (1 << i);
            else
                offset8[i / 8] &= ~(1 << (i));
        }
        break;
    }
    case READ_HOLDING_REGISTERS:

        reply->byte_count = (uint8_t)(2 * query->word_count);
        ESP_LOGI(TAG, "read_holding_registers(address: %u-%u)\n", (query->start_addr), (query->start_addr) + reply->byte_count / 2);
        for (size_t i = 0; i < reply->byte_count / sizeof(uint16_t); i++)
        {
            if (query->start_addr + i < 100) // TODO reply invalid data address in such case
            {
                offset[i] = htons(holdingRegisters[(query->start_addr) + i]);
            }
        }
        break;

    case READ_INPUT_REGISTERS:
        reply->byte_count = (uint8_t)(2 * query->word_count);
        ESP_LOGI(TAG, "read_input_registers(address: %u-%u)\n", (query->start_addr), (query->start_addr) + reply->byte_count / sizeof(uint16_t));
        for (size_t i = 0; i < reply->byte_count / sizeof(uint16_t); i++)
        {
            if (query->start_addr + i < 100) // TODO reply invalid data address in such case
            {
                offset[i] = htons(inputRegisters[(query->start_addr) + i]);
            }
        }
        break;

    case WRITE_SINGLE_COIL:
        memcpy((void *)send_buf, (void *)payload, 12);
        tmp_bytes = 3;
        break;

    case WRITE_SINGLE_REGISTER:

        holdingRegisters[query->start_addr] = htons(registers[0]);
        memcpy((void *)send_buf, (void *)payload, 12);
        reply->byte_count = 3;
        break;

    case WRITE_MULTIPLE_COILS:
        // TODO
        break;

    case WRITE_MULTIPLE_REGISTERS:
        reply->byte_count = (uint8_t)(2 * query->word_count);
        ESP_LOGI(TAG, "write_multiple_registers(address=%u, quantity=%u)", query->start_addr, reply->byte_count / sizeof(uint16_t));
        for (size_t i = 0; i < reply->byte_count / sizeof(uint16_t); i++)
        {
            if (query->start_addr + i < 100) // TODO reply invalid data address in such case
            {
                holdingRegisters[query->start_addr + i] = ((uint8_t *)payload)[13 + sizeof(uint16_t) * i] * 256 + ((uint8_t *)payload)[14 + sizeof(uint16_t) * i];
            }
        }

        memcpy((void *)send_buf, (void *)payload, 12);
        reply->byte_count = 3;
        break;

    default:
        break;
    }

    size_t to_write = sizeof(modbus_reply_t) + reply->byte_count + tmp_bytes;
    int len = to_write;
    while (to_write > 0)
    {
        int written = send(sock, send_buf + (len - to_write), to_write, 0);
        if (written < 0)
        {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        }
        to_write -= written;
    }

    delete[] send_buf;

    return ERR_OK;
}