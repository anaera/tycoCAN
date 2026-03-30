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

void initCoreCan(void);
void initPressQueue(void);
void initChip(void);
void initPins(void);
void initTMRx(void);
void i2cInit(void);
void initCAN(void);
void startTMRx(void);
void setStateIdle(void);
void initLM75(void);
void initINT(void);
void i2cOpen(void);
void canReceive(void);
void stateEngine(void);
void checkTimers(void);
void startLM75(void);
void dispatchTimer(void);
void scanPackExec(void);
void updatePins(void);
void dispatchPress(void);
void i2cClose(void);

bool app_flag(void);
bool cyc_flag(void);
bool srvChkFlag(uint8_t bitMask);

state_pin_t statePin;
packCAN_t rxCAN;
packCAN_t txCAN;
packSDO_t packSDO;
packPDO_t packPDO;

extern ee_t _ee;

void main(void)
{

	// Инициализация программных компонент
	initCoreCan(); //инициализация перем ядра CANOPEN и сервисов
	initPressQueue(); // инициализация приложения pinsControl
	// Инициализация оборудования
	initChip();
	initPins();
	initTMRx();
	i2cInit();
	initCAN();

	startTMRx();
	setStateIdle();
	initLM75();

	initINT();
	i2cOpen();
	
	while (1) {
		canReceive();
		stateEngine();
		if (app_flag()) { // опрос процессов которые не чаще 0,1сек
			checkTimers(); // Проверяет софт-таймеры - 0.1cek
			if (srvChkFlag(TMR_TEMP)) {
				 startLM75();
			}
			if (srvChkFlag(TMR_PINS)) {
				dispatchTimer();
			}
		}
		if (cyc_flag()) { // опрос процессов которые не чаще 0,01сек
			scanPackExec();			
			updatePins();
			dispatchPress();

		}
		CLRWDT();
	}
	i2cClose();
	return;
}

