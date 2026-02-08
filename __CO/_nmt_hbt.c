/*
 * File:   nmt_core.c
 * Author: Chiper
 *
 * Created on 22 декабря 2025 г., 21:43
 */


#include <xc.h>
#include "../__OD/OD.h"
#include "../_can/CAN.h"
#include "../__CO/srvCTL.h"
#include "../_epp/epp.h"

uint16_t updFlagAndTime(service_t *srvSRV);

extern __eeprom ee_t _ee;

extern uint8_t node_owner, state_node, state_send, state_prev;
extern packCAN_t rxCAN;
extern service_t srvHBT;

void setStateNMT(void)
{
	if (rxCAN.lenData == 2) {
		if ((rxCAN.data[1] == node_owner) || (rxCAN.data[1] == 0)) {
			state_prev = state_node;
			switch (rxCAN.data[0]) {
			case NMT_CMD_STOP:
				state_node = NMT_STATE_STOP;
				break;
			case NMT_CMD_OPER:
				state_node = NMT_STATE_OPER;
				break;
			case NMT_CMD_PREOP:
				state_node = NMT_STATE_PREOP;
				break;
			case NMT_CMD_RESET_COMM:
				state_node = NMT_STATE_BOOT;
				// Перезагрузка коммуникации
				break;
			case NMT_CMD_RESET_NODE:
				//			NMT_ResetNode();
				state_node = NMT_STATE_BOOT;
				RESET();
				break;
			}

		}

	}
}
bool sendEMCY(void);
bool checkSYNC(void);
bool sendPDO(void);
bool sendSDO(void);
bool sendHBT(void);
/*
bool chkState(void)
{
	if (state_node == state_send) return false;
	// перезапускаем таймер при каждом изменении статуса
	// для сетевого информирования станций
	updFlagAndTime(&srvHBT);
	return sendHBT();
}
  */
//Управление приоритетом и активностью сервисов CANOPEN
// в режиме передачи в зависимости от сотояния node_state

void packSendSelect(void)
{
	if (sendEMCY()) return;
	if (state_node != state_send) {
		updFlagAndTime(&srvHBT);
		sendHBT();
		return;
	}
	switch (state_node) {
	case NMT_STATE_OPER:
		if (checkSYNC())return;
		if (sendPDO())return;
	case NMT_STATE_PREOP:
		if (sendSDO())return;
	case NMT_STATE_STOP:
	case NMT_STATE_BOOT:
		if (sendHBT())return;
	}
}