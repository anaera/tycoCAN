#ifndef EPP_H
#define	EPP_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>

//константы canEngine

#define CAN_SPEED   2
#define GAP_TIME   6
#define RCV_MASK    0x08 
#define CRC_MASK    0x31
#define CAN_NODE 0x0E
#define START_STATE 0x03

//константы pinEngine

#define TIME_BOUNCE 5
#define TIME_QUICK 60
#define TIME_LONG 70
#define TIME_REPIT 120
#define TIME_TIMERS_WAIT 10

typedef union {
    uint8_t ctl;
 //если менять местами, то коррекция текста setPin.s (текст на асм и маски)
    struct {
        uint8_t pin : 3; //номер пина
        uint8_t reg : 1; //регистр 0 - A; 1- B 
        uint8_t inv : 1; //инверсия 0-нет; 1-есть
        uint8_t val : 1; //значение
        uint8_t dir : 1; //направление 0-вывод; 1-ввод 
        uint8_t act : 1; //тип  1-регистр; 0-память.

    };
} ctl_pin_t;

#define MSK_CYCL 0x80
#define MSK_SYNC 0x40
#define MSK_REQU 0x20

typedef union {
    uint8_t check;

    struct {
        uint8_t mAll : 1; // 1 - отправляем PDO ; 0-нет
        uint8_t mPrs : 1; // (вход)  1 - отправляем все; 0 - только значение пина
        uint8_t mUpd : 1; // (выход) 1 - отправляем все; 0- только изменение пина
        uint8_t ini3 : 1;
        uint8_t pinUPDT : 1; // по изменению состояния обекта(асинхронная передача) 
        uint8_t pinREQU : 1; //синхронизация клиента MSK_REQU(передача по запросу)
        uint8_t pinSYNC : 1; //сихронизация всех MSK_SYNC (синхронная передача))
        uint8_t pinCYCL : 1; //таймерная сихронизация MSK_CYCL(циклическая передача)
    };
} ctl_chk_t;

typedef struct {
    uint8_t fixedTime;
    uint8_t quickTime;
    uint8_t longTime;
    uint8_t repitTime;
    uint8_t timeTimers;
} time_press_t;

typedef struct {
    uint8_t actPinNum : 3;
    uint8_t action : 3;
    uint8_t stimulus : 2;
} tab_entry_t;

typedef union {
    //tab_header_t header;
    uint8_t element;
    uint8_t tab_node_owner;
    struct {
        uint8_t entPinNum : 3;
        uint8_t lenEntSec : 5;
    };
    tab_entry_t entry;
} tab_element_t;
//-----Структура для ROM

typedef union {
    uint8_t eppByte[256];

    struct {

        struct {
            uint8_t _can_speed;
            uint8_t _pack_gap;
            uint8_t _bit_mask; //маска бита выборки данных
            uint8_t _crc_poly; //полином CRC
        };

        struct {
            uint8_t _node;
            uint8_t _state;
            uint16_t _beatTime;
            uint16_t _syncTime;
            uint16_t _tempTime;
            uint16_t _pinsTime;

        };

        struct {
            ctl_pin_t ctl_pin[8];
            ctl_chk_t ctl_chk[8];
        };
        time_press_t timePress;

        tab_element_t table[16];


    };
} ee_t;

#endif	/* EPP_H */

