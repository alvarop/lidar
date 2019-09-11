#include <rplidar/rplidar.h>
#include <console/console.h>
#include <raw_uart/raw_uart.h>
#include <fifo/fifo.h>
#include "protocol.h"
#include "commands.h"
#include <stdbool.h>

#define RX_BUFF_SIZE    128

static struct os_sem sem_rx_data;

static volatile bool wait_for_rx = false;

static uint8_t rxbuff[RX_BUFF_SIZE];
static uint16_t rxbuff_idx = 0;

uint32_t send_command(uint8_t command, const void * payload, uint8_t len) {
    uint32_t rval = 0;
    rplidar_cmd_header_t header = {.sync_byte = CMD_SYNC_BYTE, .command = command};

    do {
        // Need both payload and length or neither one
        if ((payload && !len) || (!payload && len)) {
            rval = -1;
            break;
        } else if (payload && len) {
            header.command |= CMD_HAS_PAYLOAD;
        }

        raw_uart_tx((uint8_t *)&header, sizeof(header));

        if(len) {
            uint8_t checksum = 0 ^ CMD_SYNC_BYTE;
            checksum ^= command;
            checksum ^= len;

            for(uint16_t byte = 0; byte < len; byte++) {
                checksum ^= ((uint8_t *)payload)[byte];
            }

            raw_uart_tx(&len, 1);
            raw_uart_tx(&payload, len);
            raw_uart_tx(&checksum, 1);
        }

    } while(0);

    return rval;
}



void rx_handler(struct os_event *ev) {
    fifo_t *fifo = ev->ev_arg;

    while(fifo_size(fifo) && rxbuff_idx < RX_BUFF_SIZE) {
        rxbuff[rxbuff_idx++] = fifo_pop(fifo);
    }

    if(wait_for_rx) {
        wait_for_rx = false;
        os_sem_release(&sem_rx_data);
    }
}

int32_t rplidar_init() {
    console_printf("RPLidar Init\n");

    wait_for_rx = false;
    os_sem_init(&sem_rx_data, 0);

    raw_uart_init(&rx_handler);

    return 0;
}

int32_t rplidar_read(uint16_t len, os_time_t timeout) {
    os_error_t rval;

    do {
        wait_for_rx = true;
        rval = os_sem_pend(&sem_rx_data, timeout);
    } while(rval == OS_OK && rxbuff_idx < len);

    if(rval == OS_TIMEOUT) {
        console_printf("rplidar_read timeout :(\n");
        return -1;
    } else if(rval == OS_INVALID_PARM) {
        console_printf("rplidar_read invalid param :(\n");
        return -2;
    } else if(rxbuff_idx < len) {
        console_printf("rplidar_read not enough data read\n");
        return -3;
    } else {
        return 0;
    }

}

int32_t rplidar_print_info() {
    raw_uart_flush_rx();

    send_command(CMD_GET_INFO, NULL, 0);

    int32_t rval = rplidar_read(27, OS_TICKS_PER_SEC);
    if (rval) {
        console_printf("Error reading (%ld)\n", rval);
    } else {
        rplidar_rsp_header_t *header = (rplidar_rsp_header_t *)rxbuff;
        response_device_info_t *info = (response_device_info_t*)&header[1];

        if(header->sync_byte_1 == 0xA5 && header->sync_byte_2 == 0x5A) {
            console_printf("size: %ld\nsubtype:%ld\ntype: %02X\n",
                            header->size_subtype >> 2, header->size_subtype & 0x3, header->type);
            console_printf("Model: %02X\n", info->model);
            console_printf("Firmware Version: %d.%02d\n",
                            info->firmware_version >> 8, info->firmware_version & 0xFF);
            console_printf("Hardware Version: %d\n", info->hardware_version);
            console_printf("Serial Number: ");
            for(uint8_t byte = 0; byte < 16; byte++) {
                console_printf("%02X", info->serialnum[byte]);
            }
            console_printf("\n");

        }
    }

    return rval;
}
