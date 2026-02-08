/*
 * File:   _emc_snd.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 16:28
 */


#include <xc.h>
#include "../_can/CAN.h"

extern uint8_t node_error;
extern packCAN_t txCAN; //<буфер передачи

void setError(uint8_t numCode)
{
	node_error |= (uint8_t) (1 << numCode);
}

bool sendEMCY(void)
{
	if (node_error == 0) {
		return false;
	} else {
		txCAN.cmd_can = EMCY;
		txCAN.lenData = 1;
		txCAN.data[0] = node_error;
		node_error = 0;
		return true;
	}
}
