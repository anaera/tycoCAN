/*{
 * File:   srv_callBack.c
 * Author: Chiper
 *
 * Created on 29 декабря 2025 г., 18:42
 */


#include <xc.h>
#include "../__CO/srvCTL.h"
#include "../_epp/epp.h"

void updTimerHandle(uint8_t handle);
bool initTimerEntry(service_t *srvSTRUCT);
extern __eeprom ee_t _ee;

srv_t srv;

service_t srvHBT = {
	.mask = TMR_HRBT,
	.waitTime = 0,
	.handle = 0
};

service_t srvSYNC = {
	.mask = TMR_SYNC_PDO,
	.waitTime = 0,
	.handle = 0
};

service_t srvTEMP = {
	.mask = TMR_TEMP,
	.waitTime = 0,
	.handle = 0
};

service_t srvPINS = {
	.mask = TMR_PINS,
	.waitTime = 0,
	.handle = 0
};
//Требуется Маска устанавливается в прерыывании LM75
//работа с флагами сервисов в основном цикле (атомарность не требуется)

bool srvChkFlag(uint8_t bitMask)
{
	uint8_t mask = (uint8_t) (1 << (bitMask & 0x07));
	uint8_t num = bitMask >> 3;
	di();
	if (srv.byteFlag[num] & mask) {
		srv.byteFlag[num] &= (~mask);
		ei();
		return true;
	}
	ei();
	return false;
}

void srvSetFlag(uint8_t bitMask)
{
	uint8_t mask = (uint8_t) (1 << (bitMask & 0x07));
	uint8_t num = bitMask >> 3;
	di();
	srv.byteFlag[num] |= mask;
	ei();
}

bool srvGetFlag(uint8_t bitMask)
{
	uint8_t mask = (uint8_t) (1 << (bitMask & 0x07));
	uint8_t num = bitMask >> 3;
	di();
	uint8_t tmp = srv.byteFlag[num] &= mask;
	ei();
	return tmp;
}

//----HBT

void initHBT(void)
{
	srvHBT.mask = TMR_HRBT;
	srvHBT.waitTime = _ee._beatTime;
	initTimerEntry(&srvHBT); //регистрация таймера

};

void updHBT(uint16_t newTime)
{
	srvHBT.waitTime = newTime;
	updTimerHandle(srvHBT.handle);
}

uint16_t getTimeHBT(void)
{
	return srvHBT.waitTime;
}
//----TEMP

void initTEMP(void)
{
	srvTEMP.mask = TMR_TEMP;
	srvTEMP.waitTime = _ee._tempTime;
	initTimerEntry(&srvTEMP); //регистрация таймера

};

void updTEMP(uint16_t newTime)
{
	srvTEMP.waitTime = newTime;
	updTimerHandle(srvTEMP.handle);
}

uint16_t getTimeTEMP(void)
{
	return srvTEMP.waitTime;
}
// ----SYNC

void initSYNC(void)
{
	srvSYNC.mask = TMR_SYNC_PDO;
	srvSYNC.waitTime = _ee._syncTime;
	initTimerEntry(&srvSYNC); //регистрация таймера
};

void updSYNC(uint16_t newTime)
{
	srvSYNC.waitTime = newTime;
	updTimerHandle(srvSYNC.handle);
}

uint16_t getTimeSYNC(void)
{
	return srvSYNC.waitTime;
}
// ----PINS

void initPINS(void)
{
	srvPINS.mask = TMR_PINS;
	srvPINS.waitTime = _ee._pinsTime;
	initTimerEntry(&srvPINS); //регистрация таймера
};

void updPINS(uint16_t newTime)
{
	srvPINS.waitTime = newTime;
	updTimerHandle(srvPINS.handle);
}

uint16_t getTimePINS(void)
{
	return srvPINS.waitTime;
}

