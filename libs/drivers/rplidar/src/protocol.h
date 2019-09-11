#ifndef __RPLIDAR_PROTOCOL_H__
#define __RPLIDAR_PROTOCOL_H__

#include <stdint.h>

#define CMD_SYNC_BYTE               0xA5
#define CMD_HAS_PAYLOAD             0x80

#define ANS_SYNC_BYTE1              0xA5
#define ANS_SYNC_BYTE2              0x5A

#define ANS_PKTFLAG_LOOP            0x1

#define ANS_HEADER_SIZE_MASK        0x3FFFFFFF
#define ANS_HEADER_SUBTYPE_SHIFT    (30)

typedef struct {
    uint8_t sync_byte;
    uint8_t command;
} __attribute__((packed)) rplidar_cmd_header_t;

typedef struct {
    uint8_t  sync_byte_1;
    uint8_t  sync_byte_2;
    uint32_t size_subtype;
    uint8_t  type;
} __attribute__((packed)) rplidar_rsp_header_t;

#endif
