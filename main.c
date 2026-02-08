/*
 * File:   main.c
 * Author: Chiper
 *
 * Created on 22 декабря 2025 г., 21:42
 */


#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "_hard/config.h"
#include "__OD/OD.h"
#include "_can/CAN.h"
#include "_epp/epp.h"
#include "_hard/debug.h"
#include "_hard/canDrive.h"
#include "_hard/initChip.h"
#include "_hard/TMRx.h"
#include "__CO/srvCTL.h"
#include "_pins/pins.h"

packCAN_t rxCAN;
packCAN_t txCAN;
packSDO_t packSDO;
packPDO_t packPDO;

extern od_entry_t const od[];
extern ee_t _ee;
void initPressQueue(void);
void initChip(void);
void initPins(void);
void initTMRx(void);
void initCAN(uint8_t can_speed);
void startTMRx(void);
void initINT(void);
void stateEngine(void);
void serviceEngine(void);
void canSend(void);
void canReceive(void);
void initCoreCan(void);
void loadCoreApp(void);
void checkTimers(void);
void setStateIdle(void);
bool app_flag(void);
bool cyc_flag(void);
bool srvChkFlag(uint8_t bitMask);
//void clearRx(void);
//uint8_t isRxReady(void);
//bool checkSYNC(void);
//bool sendHBT(void);
//void test(void);
void i2cInit(void);
void i2cOpen(void);
void i2cClose(void);
void initLM75(void);
void startLM75(void);
void updatePins(void);
void dispatchPress(void);
void scanPackExec(void);
void dispatchTimer(void);
//void makeCMD(void);
//#define SET_GAP() 	{tmr.gapFlag = 1;TMR0 = _TMR0;}
extern uint8_t serviceFlag;
extern tmr_t tmr;
extern uint8_t state_node;
void storeActivPin(uint8_t statePin);
bool sendPDO(void);
state_pin_t statePin;

void main(void)
{
	NOP();
	//	SET_GAP();
	// Инициализация программных компонент
	initCoreCan(); //инициализация перем ядра CANOPEN и сервисов
	initPressQueue(); // инициализация приложения pinsControl
	// Инициализация оборудования
	initChip();
	initPins();
	initTMRx();
	i2cInit();
	initCAN((uint8_t) _ee._can_speed);

	startTMRx();
	setStateIdle();
	initLM75();

	initINT();
	i2cOpen();
	//state_node = NMT_STATE_OPER;
	//	test();
	//	setStateIdle();

	//	initCoreCan(); //инициализация перем ядра CANOPEN и сервисов
	//	loadCoreApp(); //инициализация перем ядра APPLICATION и сервисов
/*	NOP();
	RA0_SetDigitalOutput();
	RA1_SetDigitalOutput();
	RA0_SetHigh();
	RA1_SetHigh();
	NOP();

	//statePin.total = 0x40;	// _QP
	//storeActivPin(statePin.total);
	statePin.total = 0x40; // _QP
	storeActivPin(statePin.total);
	//statePin.total = 0xC0;	// _LP
	//storeActivPin(statePin.total);
	statePin.total = 0xC0; // _LP
	storeActivPin(statePin.total);
	
	packPDO.length = 0;
	statePin.total = 0x82; // _UP
	storeActivPin(statePin.total);
	statePin.total = 0x02; // _DN
	storeActivPin(statePin.total);
	scanExec();

//	sendPDO();

	//SET_GAP();*/
	while (1) {
		//if (isRxReady()) clearRx();
		canReceive();
		stateEngine();
		if (app_flag()) {
			checkTimers(); // Проверяет софт-таймеры - 0.1cek
			if (srvChkFlag(TMR_TEMP)) {
				 startLM75();
			}
			if (srvChkFlag(TMR_PINS)) {
				dispatchTimer();
			}
		}
		if (cyc_flag()) {
			scanPackExec();			
			updatePins();
			dispatchPress();

		}
		CLRWDT();
	}
	i2cClose();
	return;
}

