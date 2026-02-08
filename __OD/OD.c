/*
 * File:   objDir.c
 * Author: Chiper
 *
 * Created on 22 декабря 2025 г., 21:37
 */


#include <xc.h>
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

union {

	struct {
		uint8_t adr;
		uint8_t val;
	};
	uint16_t adr_val;
} epp;

uint8_t fun_val[4];

extern service_t srvHBT, srvSYNC;

// размер словаря
uint8_t od_count = 7;
// Субтаблицы
subind_t sub_1000[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &device_type}
};
subind_t sub_1001[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &error_register}
};
subind_t sub_1008[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &hard_version}
};
subind_t sub_1009[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &soft_version}
};
subind_t sub_1014[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RW), .value = &emcy}
};
subind_t sub_1017[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &srvHBT.waitTime} //период HBT
};
subind_t sub_1800[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &srvSYNC.waitTime}, // период сихронизации PDO0
	{ .num = 1, .type = T_SUB(INTEGER8, RW), .value = &ctlSYNC} //режимы сихронизации PDO0
};
subind_t sub_1f80[] = {
	{ .num = 0, .type = T_SUB(INTEGER8, RO), .value = &state_node},
	{ .num = 1, .type = T_SUB(INTEGER8, RO), .value = &state_prev}
};
subind_t sub_2000[] = {
	{ .num = 0, .type = T_SUB(INTEGER16, RW), .value = &dutyPWM} // заполнение PWM
};
subind_t sub_2001[] = {
	{ .num = 0, .type = T_SUB(FUN_EXEC, RW), .value = &fun_val}
};
subind_t sub_2002[] = {
	{ .num = 0, .type = T_SUB(EPP_RW16, RW), .value = &epp.adr_val}
};
// Object Dictionary
od_entry_t od[] = {
	{ .index = 0x1000, .sub_count = 1, .subs = &sub_1000[0]}, //device_type
	{ .index = 0x1001, .sub_count = 1, .subs = &sub_1001[0]}, //error_register
	{ .index = 0x1008, .sub_count = 1, .subs = &sub_1008[0]}, //hardware version
	{ .index = 0x1009, .sub_count = 1, .subs = &sub_1009[0]}, //software version
	{ .index = 0x1014, .sub_count = 1, .subs = &sub_1014[0]}, //EMCY
	{ .index = 0x1017, .sub_count = 1, .subs = &sub_1017[0]}, //heartbeate
	{ .index = 0x1800, .sub_count = 2, .subs = &sub_1800[0]}, //TPDO0
	{ .index = 0x1f80, .sub_count = 2, .subs = &sub_1f80[0]}, // node_state
	{ .index = 0x2000, .sub_count = 1, .subs = &sub_2000[0]}, // dutyPWM
	{ .index = 0x2001, .sub_count = 1, .subs = &sub_2001[0]}, // epp_RW
	{ .index = 0x2001, .sub_count = 1, .subs = &sub_2002[0]} // epp_RW
};

void updHBT(uint16_t newTime);
void updSYNC(uint16_t newTime);

void updPWM(uint16_t dutyPWM)
{
	if (dutyPWM > 500)return;
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
