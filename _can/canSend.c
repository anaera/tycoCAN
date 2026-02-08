/*
 * File:   canSend.c
 * Author: Chiper
 *
 * Created on 16 ноября 2025 г., 17:41
 */

#include "../_can/CAN.h"


uint8_t isTxReady(void);
void startTx(void);
uint8_t updByteCRC(uint8_t crc, uint8_t byte);


void packSendSelect(void);
void canSend(void);

extern uint8_t node_owner;
extern packCAN_t txCAN; ///<буфер передачи




#define LEN_HEAD 2

void canSend(void)
{
	uint8_t cnt = 0;
	uint8_t crc = 0;
	if (isTxReady()) { //если буфер передачи свободен
		txCAN.lenData = 0; //пытаемся сформировать пакет данных
		txCAN.rtr = 0; //пока это не пакет ответа на запрос
		packSendSelect(); //выбираем приоритетный пакет из ожидающих 
		// отправляем пакеты HBT, PDO, SDO, у всех длина <> 0!
		if (txCAN.lenData != 0) { // если были какието данные то отправляем пакет
			txCAN.hiNode = node_owner >> 3; //можно вставить номер ноды
			txCAN.loNode = node_owner & 0x07;
			for (cnt = 0; cnt < txCAN.lenData + LEN_HEAD; cnt++) { //посчитать CRC и  вставить 
				crc = updByteCRC(crc, txCAN.raw[cnt]);
			}
			txCAN.raw[txCAN.lenData + LEN_HEAD] = crc;
			startTx(); //поднимаем флаг готовности пакета
		}
	}
}




/*
#define	BIT_NMT_SERVICE 128
#define BIT_EMCY_SERVICE  64
#define BIT_SYNC_SERVICE  32
#define BIT_TIME_SERVICE  16
#define BIT_PDO_SERVICE  8
#define BIT_SDO_SERVICE  4
#define BIT_HBT_SERVICE  2


#define	MASK_BOOT_STATE  BIT_HBT_SERVICE
#define MASK_STOP_STATE  BIT_HBT_SERVICE | BIT_EMCY_SERVICE
#define	MASK_PREP_STATE  BIT_HBT_SERVICE | BIT_SDO_SERVICE| BIT_EMCY_SERVICE 
#define	MASK_OPER_STATE  BIT_HBT_SERVICE | BIT_SDO_SERVICE | BIT_PDO_SERVICE| BIT_EMCY_SERVICE  

//enum {	MASK_OPER_STATE = BIT_HBT_SERVICE | BIT_EMCY_SERVICE | BIT_SDO_SERVICE | BIT_PDO_SERVICE};

uint8_t const maskServicePriority[4] = {MASK_BOOT_STATE, MASK_STOP_STATE, MASK_PREP_STATE, MASK_OPER_STATE};

bool(* const prioritySendService[8])(void) = {sendSkip, sendEMC, sendSkip, sendSkip, sendPDO, sendSDO, sendHBT, sendSkip};

void packSendSelect(void)
{
	uint8_t cnt = 0;
	uint8_t maskAct;
	maskAct = 0x80;
	while (maskAct) {
		if (maskServicePriority[core.state] & maskAct) {
			if (prioritySendService[cnt]())return;
		}
		cnt++;
		maskAct = (uint8_t) maskAct >> 1;
	}
	return;
}
 */