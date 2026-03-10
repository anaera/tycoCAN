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

#define T_SUB(X, Y) ((X) | ((Y) << 6))

// Пример данных
uint32_t error_register = 0x77;
uint16_t dutyPWM = 125;
uint8_t device_type = 0xAA;
uint8_t hard_version = 0x11;
uint8_t soft_version = 0x11;
uint8_t emcy;
uint8_t state_node, state_send, state_prev;
uint8_t ctlSYNC;
uint8_t node_owner;
uint8_t node_error;
uint8_t tmp_Val, tmp_Max, tmp_Min, tmp_Pin;

epp_sdo_t epp_sdo;

uint8_t fun_val[4];

extern service_t srvHBT, srvSYNC;

// размер словаря
#define OD_MAX 12

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
	{ .num = 1, .type = T_SUB(INTEGER8, RW), .value = &ctlSYNC} //режимы сихронизации PDO0
};
const subind_t sub_1f80[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &state_node},
	{ .num = 1, .type = T_SUB(INTEGER8, RO), .value = &state_prev}
};
const subind_t sub_2000[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &dutyPWM} // заполнение PWM
};
const subind_t sub_2001[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &tmp_Val},
	{ .num = 1, .type = T_SUB(INTEGER8, RW), .value = &tmp_Max},
	{ .num = 2, .type = T_SUB(INTEGER8, RW), .value = &tmp_Min},
	{ .num = 3, .type = T_SUB(INTEGER8, RW), .value = &tmp_Pin}
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
	{ .index = 0x1017, .sub_count = 1, .subs = &sub_1017[0]}, //heartbeate
	{ .index = 0x1800, .sub_count = 2, .subs = &sub_1800[0]}, //TPDO0
	{ .index = 0x1f80, .sub_count = 2, .subs = &sub_1f80[0]}, // node_state
	{ .index = 0x2000, .sub_count = 1, .subs = &sub_2000[0]}, // dutyPWM
	{ .index = 0x2001, .sub_count = 4, .subs = &sub_2001[0]}, // temp00
	{ .index = 0x2201, .sub_count = 1, .subs = &sub_2201[0]}, // fun_light
	{ .index = 0x2202, .sub_count = 2, .subs = &sub_2202[0]} // epp_RW
};

uint8_t count_elements = sizeof(od) / sizeof(od[0]);

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

void checkTemp1(uint8_t tempVal)
{
	state_pin_t statePin;
	if (tempVal > tempMax || tempVal < tempMin) {
		statePin.number = tempPin;
		statePin.set.val = (tempVal > tempMax);
		writePin(statePin);
	}
}
*/
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
