/*
 * File:   _sync_rcv.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 16:28
 */
#include <xc.h>
#include "../_can/CAN.h"
#include "../__CO/srvCTL.h"

extern packCAN_t rxCAN; ///<буфер приема
extern uint8_t node_owner;

void srvSetFlag(uint8_t mask);

void setSync(void)
{
	if (rxCAN.lenData == 1) {
		if (rxCAN.data[0] == node_owner) {
			srvSetFlag(SYS_NODE_SYNC);
		}
		if (rxCAN.data[0] == 0) {
			srvSetFlag(SYS_MASTER_SYNC);
		}
	}
}
