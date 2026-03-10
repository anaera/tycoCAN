#ifndef CO_H
#define	CO_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

//consumer - прием
#define NMT     0x00 
#define SYN     0x01
#define TIM     0x02
//produser - передача

#define EMCY    0x01
#define PDO0    0x03
#define PDO1    0x04
#define PDO2    0x05
#define PDO8    0x0A
#define SDOR    0x0B
#define SDOA    0x0C
//#define SDOP    0x0D
#define HRBT    0x0E

//CMD_SPEC

#define UPL_REQU 2  // 0b(010)0 0000
#define UPL_RESP 3  // 0b(011)n nnnn
#define DNL_REQU 6  // 0b(110)n nnnn
#define DNL_RESP 7  // 0b(111)0 0000
#define ERR_RESP 4  // 0b(100)0 0000

//SDO_error_code
#define ERR_UNKNOW_SDO 0x0400
#define ERR_INDEX 0x0401
#define ERR_SUBINDEX 0x0402
#define ERR_RO_FAULT 0x0403
#define ERR_WR_FAULT 0x0404
#define ERR_WRONG_TYPE 0x0405
#define ERR_UNKNOW_TYPE 0x0406



// Состояния STATE_MACHINE
#define NMT_STATE_BOOT    0
#define NMT_STATE_STOP    1
#define NMT_STATE_PREOP   2
#define NMT_STATE_OPER    3

// Команды NMT
#define NMT_CMD_STOP      0x02
#define NMT_CMD_OPER      0x01
#define NMT_CMD_PREOP     0x80
#define NMT_CMD_RESET_COMM 0x81
#define NMT_CMD_RESET_NODE 0x82

// Состояния STATE_SEND
#define HBT_SEND_BOOT  0
#define HBT_SEND_STOP  4
#define HBT_SEND_PREP  127
#define HBT_SEND_OPER  5

// SDO-пакет

typedef union {
    uint8_t raw[8];

    struct {

        union {
            uint8_t cmd_spec;

            struct {
                uint8_t len_value : 5;
                uint8_t reqeue_type : 3; // transfer type
            };
        };

        union {
            uint16_t index_sdo;
        };
        uint8_t subindex_sdo;

        union {
            uint8_t val08t[4];
            uint16_t val16t[2];
            uint32_t val32t[1];
            uint32_t error_code;
        };
    };
} packSDO_t;
//HBT пакет

typedef union {
    uint8_t raw[2];

    struct {
        uint8_t nmt_cmd;
        uint8_t node;
    };
} packHBT_t;

//HBT пакет

typedef struct {
    uint8_t length;
    uint8_t raw[8];
} packPDO_t;

//HBT пакет

typedef struct {
    uint8_t raw[1];
} packEMCY_t;
#endif