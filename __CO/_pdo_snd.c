/*
 * File:   _pdo_snd.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 16:26
 */

#include <xc.h>
#include "../_can/CAN.h"
#include "../__CO/CO.h"
#include "../__CO/srvCTL.h"
#include "../_pins/pins.h"
#include "../_epp/epp.h"

bool srvChkFlag(uint8_t mask);
uint8_t getTempByte(uint8_t length);


extern packCAN_t txCAN; ///<буфер передачи
extern packPDO_t packPDO;
extern ctl_pin_t ctl_pin[8];
extern __eeprom ee_t _ee;

/*void addPinPDO0(uint8_t addPin)
{
	state_pin_t statePin;
	statePin.total = addPin;
	uint8_t number = statePin.number;
	ctl_chk_t check = _ee.ctl_chk[number];

	if (!check.mAll) {
		return; // Не отправляем сообщение, если это пин "молчания"
	}
	if (ctl_pin[number].dir) {
		// Пин является входом
		if (statePin.set.act == 1 && !check.mPrs) {
			return; // Press не отправляется
		}
	} else {
		// Пин является выходом
		if (ctl_pin[number].val == statePin.set.val && !check.mUpd) {
			return; // Нет необходимости отправлять обновление
		}
	}
	txCAN.data[txCAN.lenData] = statePin.total;
	txCAN.lenData++;
}*/

bool makePDO0(void)
{
	for (uint8_t cnt = 0; cnt < packPDO.length; cnt++) {
		//addPinPDO0(packPDO.raw[cnt]);
		txCAN.data[txCAN.lenData] = packPDO.raw[cnt]; // из буфера
		packPDO.raw[cnt] = 0; //это лишнее??	
		txCAN.lenData++; // следующий
	}
	packPDO.length = 0; //буфер чист
	if (txCAN.lenData == 0)return false;
	txCAN.cmd_can = PDO0; // Установили команду
	return true;
}

bool makePDO1(void)
{
	if (srvChkFlag(SYS_TEMP_PDO)) {
		//		txCAN.lenData = 0; // лишнее

		while (txCAN.lenData < 8) { //LEN_DATA
			txCAN.data[txCAN.lenData] = getTempByte(txCAN.lenData);
			txCAN.lenData++;
			txCAN.data[txCAN.lenData] = getTempByte(txCAN.lenData);
			txCAN.lenData++;
		}
		txCAN.cmd_can = PDO1; // Установили команду
		return true;
	}
	return false;
}

bool sendPDO(void)
{
	if (makePDO0()) return true;
	if (makePDO1()) return true;
	return false;
}
