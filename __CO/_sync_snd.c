/*
 * File:   _sync_snd.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 16:29
 */


#include <xc.h>
#include "../__CO/srvCTL.h"
#include "../_epp/epp.h"
#include "../_can/CAN.h"
#include "../_pins/pins.h"

bool checkSYNC(void);
uint8_t makePack(uint8_t mask);
void updTimerHandle(uint8_t handle);
bool initTimerEntry(service_t *srvSTRUCT);
uint8_t readPin(uint8_t numPin);
bool srvChkFlag(uint8_t mask);
uint8_t getControlPin(uint8_t num);


extern packCAN_t txCAN; ///<буфер передачи

uint8_t makePack(uint8_t mask)
{
	state_pin_t statePin;
	statePin.total = 0;
	txCAN.cmd_can = PDO0;
	txCAN.lenData = 0;
	for (uint8_t cnt = 0; cnt < MAX_PINS; cnt++) {
		if (getControlPin(cnt) & mask) { //должен ли бит отслеживаться 
			statePin.set.val = readPin(cnt);
			statePin.number = cnt;
			txCAN.data[txCAN.lenData] = statePin.total; //readPin(uint8_t cnt);
			txCAN.lenData++;
		}
	}
	return txCAN.lenData;
}

bool checkSYNC(void)
{
	if (srvChkFlag(TMR_SYNC_PDO)) {
		if (makePack(MSK_CYCL)) return true; //циклически
	}
	if (srvChkFlag(SYS_MASTER_SYNC)) {
		if (makePack(MSK_SYNC)) return true; //по MASTER SYNC 
	}
	if (srvChkFlag(SYS_NODE_SYNC)) {
		if (makePack(MSK_REQU)) return true; // по NODE SYNC
	}
	return false;
}

