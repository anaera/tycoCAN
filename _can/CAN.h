#ifndef CAN_H
#define CAN_H

#include "../__CO/CO.h"

// CAN-пакет

typedef union {
    uint8_t raw[11];

    struct {

        union {
            uint16_t canHead;

            struct {
                uint8_t hiHead;
                uint8_t loHead;
            };

            struct {
                uint8_t hiNode : 4;
                uint8_t cmd_can : 4;

                uint8_t lenData : 4;
                uint8_t rtr : 1;
                uint8_t loNode : 3;
            };
        };

        union {
            packSDO_t packSDO;
            uint8_t data[8];
        };

        uint8_t crc;

    };
} packCAN_t;


#endif