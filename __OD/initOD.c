/*
 * File:   coreCan.c
 * Author: Chiper
 *
 * Created on 19 ноября 2025 г., 13:50
 */


#include <xc.h>


#include "../_epp/epp.h"
#include "../_hard/debug.h"
#include "../__CO/CO.h"

extern __eeprom ee_t _ee;
extern uint8_t node_owner, node_error;

extern uint8_t state_node, state_prev, state_send;

//static errorStruct_t errorCount;
// Глобальный флаг сервисов (необязательно volatile)

// Прототипы инициализации 
void initCoreCan(void);
void initSYNC(void);
void initHBT(void);
void initTEMP(void);
void initPINS(void);
//void initAPP(void);
//void initSDO(void);

void initSrvCore(void)
{
	initHBT();
	initSYNC();
	initTEMP();
	initPINS();
	//	initAPP();
}

void initCoreCan(void)
{
	node_owner = _ee._node;
	node_error = 0;
	state_node = 0;
	state_prev = 0;
	state_send = 0xFF;
	initSrvCore();
}



