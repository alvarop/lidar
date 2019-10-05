#include <rplidar/rplidar.h>
#include <console/console.h>
#include <bsp/bsp.h>
#include <hal/hal_gpio.h>
#include <raw_uart/raw_uart.h>
#include <fifo/fifo.h>
#include <stdbool.h>
#include "protocol.h"
#include "commands.h"

#define RX_BUFF_SIZE    128

static struct os_sem sem_rx_data;
static volatile bool wait_for_rx = false;
static uint8_t rxbuff[RX_BUFF_SIZE];
static fifo_t *rx_fifo;

#define BINS 360

uint16_t distances[BINS];

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
    // fifo_t *fifo = ev->ev_arg;
    hal_gpio_write(MCU_GPIO_PORTB(1), 1);
    if(wait_for_rx) {
        wait_for_rx = false;
        os_sem_release(&sem_rx_data);
    }
    hal_gpio_write(MCU_GPIO_PORTB(1), 0);
}

void rplidar_enable_motor() {
    hal_gpio_write(MCU_GPIO_PORTC(4), 1);
}

void rplidar_disable_motor() {
    hal_gpio_write(MCU_GPIO_PORTC(4), 0);
}

int32_t rplidar_init() {
    console_printf("RPLidar Init\n");

    hal_gpio_init_out(MCU_GPIO_PORTC(4), 0);

    wait_for_rx = false;
    os_sem_init(&sem_rx_data, 0);

    raw_uart_init(&rx_handler);
    raw_uart_get_rx_fifo(&rx_fifo);

    rplidar_stop_scan();

    return 0;
}

int32_t rplidar_read(uint8_t *buff, uint16_t len, os_time_t timeout) {
    os_error_t rval;

    do {
        wait_for_rx = true;
        rval = os_sem_pend(&sem_rx_data, timeout);
    } while(rval == OS_OK && fifo_size(rx_fifo) < len);

    if(rval == OS_TIMEOUT) {
        console_printf("rplidar_read timeout :(\n");
        return -1;
    } else if(rval == OS_INVALID_PARM) {
        console_printf("rplidar_read invalid param :(\n");
        return -2;
    } else if(fifo_size(rx_fifo) < len) {
        console_printf("rplidar_read not enough data read\n");
        return -3;
    } else if (len > sizeof(rxbuff)) {
        return -4;
    } else {
        while(len--) {
            *buff++ = fifo_pop(rx_fifo);
        }

        return 0;
    }

}

int32_t rplidar_validate_response(uint32_t *len, uint8_t *subtype, uint8_t *type) {
    int32_t rval;

    fifo_flush(rx_fifo);

    rval = rplidar_read(rxbuff, 7, OS_TICKS_PER_SEC);

    if(rval == 0) {
        rplidar_rsp_header_t *header = (rplidar_rsp_header_t *)rxbuff;
        if(len) {
            *len = header->size_subtype & ANS_HEADER_SIZE_MASK;
        }
        if(subtype) {
            *subtype = header->size_subtype >> ANS_HEADER_SUBTYPE_SHIFT;
        }
        if(type) {
            *type = header->type;
        }
    }

    return rval;
}

int32_t rplidar_print_info() {

    send_command(CMD_GET_INFO, NULL, 0);

    uint32_t len = 0;
    uint32_t rval;
    do {
        rval = rplidar_validate_response(&len, NULL, NULL);
        if(rval) {
            console_printf("Invalid response (%ld)\n", rval);
            break;
        } else if(len != 20) {
            console_printf("Invalid response data len (%ld)\n", len);
            rval = -1;
            break;
        }

        int32_t rval = rplidar_read(rxbuff, 20, OS_TICKS_PER_SEC);
        if (rval) {
            console_printf("Error reading (%ld)\n", rval);
            break;
        } else {

            response_device_info_t *info = (response_device_info_t*)rxbuff;

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
    } while (0);

    return rval;
}

int32_t rplidar_start_scan() {
    send_command(CMD_SCAN, NULL, 0);

    uint32_t len = 0;
    uint32_t rval;
    rval = rplidar_validate_response(&len, NULL, NULL);

    if(rval == 0) {
        // console_printf("Scan started...%ld\n", len);
    } else {
        console_printf("Error starting scan %ld\n", rval);
    }

    return rval;
}

int32_t rplidar_stop_scan() {
    send_command(CMD_STOP, NULL, 0);

    // console_printf("Scan stopped...\n");

    return 0;
}


int32_t rplidar_run() {

    bool scanning = true;
    int32_t rval;
    int16_t current_angle = -1;
    uint32_t current_distance = 0;

    while(scanning) {
        rval = rplidar_read(rxbuff, 5, OS_TICKS_PER_SEC);

        if(rval) {
            break;
        }

        rplidar_scan_packet_t * packet = (rplidar_scan_packet_t *)rxbuff;

        uint8_t check = packet->quality & 0x3;
        // uint8_t quality = packet->quality >> 2;

        if (check == 2) {
            uint16_t angle = (packet->angle >> 1)/64;
            uint16_t distance = packet->distance/4;

            if(angle < BINS) {
                if (angle == current_angle) {
                    // Average both measurements
                    current_distance += distance;
                    current_distance /= 2;
                    distance = current_distance;
                } else {
                    current_distance = 0;
                }

                distances[angle] = distance;
                current_angle = angle;
            } else {
                // discard
            }
        }
    }

    return rval;
}
