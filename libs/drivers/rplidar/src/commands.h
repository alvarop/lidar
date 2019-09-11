#ifndef __RPLIDAR_COMMANDS_H__
#define __RPLIDAR_COMMANDS_H__

#include <stdint.h>

#define CMD_STOP            0x25
#define CMD_RESET           0x40
#define CMD_SCAN            0x20

#define CMD_GET_INFO        0x50
#define CMD_GET_HEALTH      0x52

typedef struct {
    uint8_t     model;
    uint16_t    firmware_version;
    uint8_t     hardware_version;
    uint8_t     serialnum[16];
} __attribute__((packed)) response_device_info_t;

#endif
