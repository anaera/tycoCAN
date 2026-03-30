/*
 * File:   objDir.c
 * Author: Chiper
 *
 * Created on 22 декабря 2025 г., 21:37
 */


#include <xc.h>
#include "../_pins/pins.h"
#include "../__OD/OD.h"
#include "../__CO/srvCTL.h"
#include "../_lm75//lm75.h"

#define T_SUB(X, Y) ((X) | ((Y) << 6))

// Пример данных
uint32_t error_register = 0x77;
uint16_t dutyPWM = 125;
uint8_t device_type = 0xAA;
uint8_t hard_version = 0x11;
uint8_t soft_version = 0x11;
uint8_t emcy;
uint8_t state_node, state_send, state_prev;
uint8_t node_owner;
uint8_t node_error;

uint8_t by_event, by_request, by_timer;

epp_sdo_t epp_sdo;
uint8_t fun_val[4];

temp_t tempVal; //пакет температуры для отсылки
int8_t tmp_Max, tmp_Min;
uint8_t num_termo;

extern service_t srvHBT, srvSYNC;

// размер словаря
#define OD_MAX 13
//uint8_t count_elements = sizeof(od) / sizeof(od[0]);
uint8_t od_count = OD_MAX;

// Субтаблицы
const subind_t sub_1000[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &device_type}
};
const subind_t sub_1001[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &error_register}
};
const subind_t sub_1008[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &hard_version}
};
const subind_t sub_1009[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &soft_version}
};
const subind_t sub_1014[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RW), .value = &emcy}
};
const subind_t sub_1017[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &srvHBT.waitTime} //период HBT
};
const subind_t sub_1800[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &srvSYNC.waitTime}, // период сихронизации PDO0
	{ .num = 1, .type = T_SUB(INTEGER8, RW), .value = &by_event}, //сихронизация по событию
	{ .num = 2, .type = T_SUB(INTEGER8, RW), .value = &by_request}, //сихронизация по запросу
	{ .num = 3, .type = T_SUB(INTEGER8, RW), .value = &by_timer} //сихронизация по периоду
};
const subind_t sub_1f80[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &state_node},
	{ .num = 1, .type = T_SUB(INTEGER8, RO), .value = &state_prev}
};
const subind_t sub_2000[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &dutyPWM} // заполнение PWM
};
const subind_t sub_2001[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RO), .value = &tempVal.temp16t[0]},
	{ .num = 1, .type = T_SUB(INTEGER16, RW), .value = &tempVal.temp16t[1]},
	{ .num = 2, .type = T_SUB(INTEGER16, RW), .value = &tempVal.temp16t[2]},
	{ .num = 3, .type = T_SUB(INTEGER16, RW), .value = &tempVal.temp16t[3]}
};
const subind_t sub_2002[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RW), .value = &num_termo},
	{ .num = 1, .type = T_SUB(INTEGER8, RW), .value = &tmp_Max},
	{ .num = 2, .type = T_SUB(INTEGER8, RW), .value = &tmp_Min}

};
const subind_t sub_2201[] = {
	{ .num = 0, .type = T_SUB(FUN_EXEC, WO), .value = &fun_val}
};
const subind_t sub_2202[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RW), .value = &epp_sdo.address},
	{ .num = 1, .type = T_SUB(EPP_RW16, RW), .value = &epp_sdo.adr_val}
};
// Object Dictionary
const od_entry_t od[] = {
	{ .index = 0x1000, .sub_count = 1, .subs = &sub_1000[0]}, //device_type
	{ .index = 0x1001, .sub_count = 1, .subs = &sub_1001[0]}, //error_register
	{ .index = 0x1008, .sub_count = 1, .subs = &sub_1008[0]}, //hardware version
	{ .index = 0x1009, .sub_count = 1, .subs = &sub_1009[0]}, //software version
	{ .index = 0x1014, .sub_count = 1, .subs = &sub_1014[0]}, //EMCY
	{ .index = 0x1017, .sub_count = 1, .subs = &sub_1017[0]}, //период heartbeate
	{ .index = 0x1800, .sub_count = 4, .subs = &sub_1800[0]}, //параметры TPDO0
	{ .index = 0x1f80, .sub_count = 2, .subs = &sub_1f80[0]}, // режим работу устр-ва (nodeState)
	{ .index = 0x2000, .sub_count = 1, .subs = &sub_2000[0]}, // dutyPWM
	{ .index = 0x2001, .sub_count = 4, .subs = &sub_2001[0]}, // температура с датчиков
	{ .index = 0x2002, .sub_count = 3, .subs = &sub_2002[0]}, // установки термостата
	{ .index = 0x2201, .sub_count = 1, .subs = &sub_2201[0]}, // команды управления светом
	{ .index = 0x2202, .sub_count = 2, .subs = &sub_2202[0]} // чтение запись EPPROM
};


void updHBT(uint16_t newTime);
void updSYNC(uint16_t newTime);

/*
void writePin(state_pin_t statePin);

void checkTemp0(uint8_t tempVal) 
{
	state_pin_t statePin;
	statePin.number = tempPin;

	if (tempVal > tempMax) {
		statePin.set.val = 1;
	} else {
		if (tempVal < tempMin) {
			statePin.set.val = 0;
		} else {
			return;
		}
	}
	statePin.number = tempPin;
	writePin(statePin);
}
 */
void writePin(state_pin_t statePin);

void termostat(uint8_t num)
{
	const uint8_t num_pin[4] = {7, 6, 3, 4};
	state_pin_t statePin;
	if (num < num_termo) {
		if (tempVal.temp16t[num] > tmp_Max || tempVal.temp16t[num] < tmp_Min) {
			statePin.number = num_pin[num];
			statePin.set.val = (tempVal.temp16t[num] > tmp_Max);
			writePin(statePin);
		}
	}
}

void updPWM(uint16_t newDutyPWM)
{
	if (newDutyPWM > 500)return;
	dutyPWM = newDutyPWM;
	CCPR2L = (uint8_t) ((dutyPWM & 0x01FF) >> 1);
};
// Массив пар "Адрес переменной - Обработчик"
// Регистрация переменных и обработчиков
const map_entry_t map[MAP_SIZE] = {
	{&srvHBT.waitTime, updHBT},
	{&srvSYNC.waitTime, updSYNC},
	{&dutyPWM, updPWM}
};

void srv_callBack(uint16_t *oldVal, uint16_t newVal)
{
	uint16_t *temp;
	if (*oldVal == newVal) return; //если не изменилась
	for (uint8_t cnt = 0; cnt < MAP_SIZE; cnt++) {
		temp = map[cnt].var_addr;
		if (oldVal == temp) {
			map[cnt].handler(newVal);
			return;
		}
	}
};
