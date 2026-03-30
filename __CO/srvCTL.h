#ifndef SRV_H
#define	SRV_H


#include <xc.h> // include processor files - each processor file is guarded.  


/*#define CYCL_SYNC_MSK 0x01
#define HBT_MASK 0x02
#define APP_MASK 0x04
#define TEMP_MASK 0x08
 * 
#define MASTER_SYNC_MSK 0x10
#define NODE_SYNC_MSK 0x20
#define PDO0_MASK 0x40
#define SDO_MASK 0x80*/

// флаги сервисов таймера
#define TMR_HRBT 0 
#define TMR_SYNC_PDO 1
#define TMR_TEMP 2
#define TMR_PINS 3

// флаги сервисов canOPEN
#define SYS_MASTER_SYNC 8
#define SYS_TERMOSTAT 9
#define SYS_ANS_SDO  10
#define SYS_TEMP_PDO 11

typedef union {
	uint16_t serviceFlags;

	struct {
		uint8_t byteFlag[2];
	};

	struct {

		struct {
			uint8_t tmrHTBT : 1; //0
			uint8_t tmrSYNC : 1; //1
			uint8_t tmrTEMP : 1; //2
			uint8_t tmrQUIT : 1; //3
			uint8_t tmr0004 : 1; //4
			uint8_t tmr0005 : 1; //5
			uint8_t tmr0006 : 1; //6
			uint8_t tmr0007 : 1; //7
		};

		struct {
			uint8_t sysM_SYNC  : 1; //8
			uint8_t sysTERMO  : 1; //9
			uint8_t sysANS_SDO : 1; //10
			uint8_t sysTMP_PDO : 1; //11
			uint8_t sys0012 : 1; //12
			uint8_t sys0013 : 1; //13
			uint8_t sys0014 : 1; //14
			uint8_t sys0015 : 1; //15
		};
	};
} srv_t;


// Структура управления сервиса таймера
typedef struct {
    uint8_t mask; // Битовая маска в serviceFlag
    uint16_t waitTime; // Период ожидания (такты)
    uint8_t handle; // Хэндл (управление таймером)
} service_t;


//количество входов в таблице адресов OD
#define MAP_SIZE 4
// Структура пары: адрес переменной и указатель на функцию-обработчик
typedef struct {
    uint16_t *var_addr; // Адрес переменной
    void (*handler)(uint16_t new_val); // Указатель на функцию-обработчик
} map_entry_t;


// Макрос: максимальное количество таймеров (ограничено ресурсами)
#define MAX_TIMERS      4
// Структура одного таймерного входа
typedef struct {
    uint16_t coutTime; // Оставшееся время до срабатывания
    service_t *srvStruct; // адрес структуры управления сервисом
} timer_entry_t;



#endif	/* SRV_H */

