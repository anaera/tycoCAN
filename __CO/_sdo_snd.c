/*
 * File:   _sdo_snd.c
 * Author: Chiper
 *
 * Created on 26 декабря 2025 г., 13:31
 */


#include "../_can/CAN.h"
#include "../__CO/srvCTL.h"

extern packCAN_t txCAN;
extern packSDO_t packSDO;
extern bool srvChkFlag(uint8_t mask);

bool sendSDO(void)
{
	if (srvChkFlag(SYS_ANS_SDO)) {
		txCAN.cmd_can = SDOA;
		txCAN.lenData = 8;
		for (uint8_t cnt = 0; cnt < txCAN.lenData; cnt++) {
			txCAN.data[cnt] = packSDO.raw[cnt]; //sdo
		}
		return true;
	}
	return false;
}

