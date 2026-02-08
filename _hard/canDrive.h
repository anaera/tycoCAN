/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */
#ifndef CANDRIVE_H
#define	CANDRIVE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>

#define CAN_LEN_HEADER 2
#define CAN_LEN_BUFFER 12
#define TxRETRY 7
#define SEND_BYTE_HI 0xFC
#define SEND_BYTE_LO 0xC0

uint8_t updByteCRC(uint8_t crc, uint8_t byte);
uint8_t updBitCRC(uint8_t crc, uint8_t bit);
void initCAN(uint8_t num);
void loadConstEngine(void);
void initNewCycle(void);
inline void setStateSend(void);
inline void setStateReceive(void);
inline void setStateIdle(void);
//inline uint8_t nextBit(void);
inline void checkAndActSendPack(void);
void stateEngine(void);
inline void packActEngine(void);
uint8_t isReadyGet(void);
uint8_t isReadyPut(void);
void setEcho(void);
void clrEcho(void);

/**
 * Машина состояний приёма/передачи:
 * 
 * IDLE_STEP  - ожидание активности шины (INT0)
 * RECV_STEP  - приём пакета (выигран арбитраж или пассивный узел)
 * SEND_STEP  - передача пакета (активный узел)
 * SKIP_STEP  - ошибка: доприём до таймаута, затем переход в IDLE
 */
enum {
    IDLE_STEP = 0x00,
    RECV_STEP = 0x01,
    SEND_STEP = 0x02,
    SKIP_STEP = 0x03
};

enum {
    eUART = 0, // 0x01  ошибока UART
    eSEND = 1, // 0x02  ошибока арбитража байта
    eLEN = 2, // 0x04  ошибока размера пакета
    eCRC = 3, // 0x08  ошибока CRC
    eRxD = 4, // 0x10  ошибока переполнения буфера приема RxD 

    eTxD = 5, // 0x20  ошибока передачи пакета
    ePDO = 6, // 0x40  ошибока переполнения буфера PDO
    eARBIT = 7, // 0x80  проигран арбитраж

};

typedef union {
    uint8_t allFlags;

    struct {
        uint8_t sysFlag : 1;
        uint8_t cycFlag : 1;
        uint8_t appFlag : 1;
        uint8_t gapFlag : 1;
    };
} tmr_t;

typedef struct {
    uint8_t gapTime; ///<величина защитного интервала в микросекундах
    uint8_t rcvMask; ///<маска выбоки бита 
    uint8_t crcMask; ///<CRC полином
} constant_t;

typedef union {
    uint8_t flags;

    struct {
        uint8_t rcvIsFull : 1; //пакет принят полностью
        uint8_t sndIsFull : 1; //пакет готов к отправке 
        uint8_t sndCounter : 3; //счетчик повторных попыток отправки
        uint8_t sndBit : 1; // отправляемый бит
        uint8_t rcvBit : 1; // принятый бит 
        uint8_t echoTX : 1; // режим эхо приема
    };
}
stateDriver_t;

typedef struct {
    //пакетные счетчики бит, байт, контроля размера пакета
    uint8_t lenPack; ///<длина принимаемого пакета в байтах 0-8
    uint8_t lenData; ///<длина блока данных в байтах  1-9  // макс 9=8(datа)+1(crc)
    uint8_t ptrPack; ///<счетчик/номер принятого бита в паккете 0-80
    uint8_t cntPack; ///<счетчик/номер байт в пакете        0-10 //11 байт
    uint8_t crcByte; ///<байт контрольной суммы


    uint8_t sndByte; ///<передаваемый байт
    uint8_t rcvByte; ///<принимаемый байт

    uint8_t bitMask; ///<маска позиции бита в байте
} controlDrive_t;

typedef union {
    uint8_t all[8];

    struct {
        uint8_t drv; //ошибока UART
        uint8_t bus; //ошибока передачи байта
        uint8_t crc; //ошибока CRC

        uint8_t rpt; //ошибока передачи пакета
        uint8_t ovr; //ошибока переполнения буфера приема RxD
        uint8_t pdo; //ошибока переполнения буфера PDO

        uint8_t re1; //резерв
        uint8_t re2; //резерв
    };
}
errorCount_t;
#endif	/* CANDRIVE_H */

