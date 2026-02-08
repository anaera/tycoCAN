/*
 * File:   _hbt_snd.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 16:25
 */


#include <xc.h>
#include "../_can/CAN.h"
#include "../__CO/srvCTL.h"
#include "../_epp/epp.h"

bool srvChkFlag(uint8_t mask);

const uint8_t stateName[4] = {HBT_SEND_BOOT, HBT_SEND_STOP, HBT_SEND_PREP, HBT_SEND_OPER};

extern __eeprom ee_t _ee;
extern packCAN_t txCAN; ///<буфер передачи
extern uint8_t state_node, state_send;

bool sendHBT(void)
{
	if (srvChkFlag(TMR_HRBT)) {

		state_send = state_node;
		txCAN.cmd_can = HRBT;
		txCAN.lenData = 1;
		txCAN.data[0] = stateName[state_send];

		if (state_node == NMT_STATE_BOOT) {
			state_node = _ee._state;
		}

		return true;
	}
	return false;
}





