/*
 * File:   CanReceive.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 19:16
 */

#include <xc.h>
#include "../_can/CAN.h"

extern packCAN_t rxCAN; ///<буфер приема
extern uint8_t node_owner, state_node;


uint8_t isRxReady(void);

void execSDO(void);
void setStateNMT(void);
void setSync(void);
void setTime(void);
void clearRx(void);
void canReceive(void);

bool handleOperState(uint8_t node);
bool handlePrepState(uint8_t node);
bool handleStopState(uint8_t node);

void pdoExec(uint8_t node);

//Управление приоритетом и активностью сервисов CANOPEN в режиме приема
//в зависимости от сотояния coreCan.state

void canReceive(void)
{
	uint8_t node;
	if (isRxReady()) {
		node = ((rxCAN.loNode) + (uint8_t) (rxCAN.hiNode << 3));
		switch (state_node) {
		case NMT_STATE_OPER: // State OPERATION
			if (handleOperState(node)) {
				break;
			}
		case NMT_STATE_PREOP: // State PREPARE
			if (handlePrepState(node)) {
				break;
			}
		case NMT_STATE_STOP: // State STOP
		case NMT_STATE_BOOT: // State BOOT
			if (handleStopState(node)) {
				break;
			}
		default:

			break;
		}
		clearRx();
	}
}

bool handleStopState(uint8_t node)
{
	if (node == 0) {
		switch (rxCAN.cmd_can) {
		case NMT:
			setStateNMT();
			return true;
		case TIM:
			setTime();
			return true;
		default:
			return false;
		}
	}
	return false;
}

bool handlePrepState(uint8_t node)
{
	if (node == node_owner) {
		if (rxCAN.cmd_can == SDOR) {
			execSDO();
			return true;
		}
	}
	return false;
}

bool handleOperState(uint8_t node)
{

	switch (rxCAN.cmd_can) {
	case PDO0:
		pdoExec(node);
		return true;
	case SYN:
		setSync();
		return true;
	default:
		return false;
	}
}