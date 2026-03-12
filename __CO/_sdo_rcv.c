/*
 * File:   objDir.c
 * Author: Chiper
 *
 * Created on 22 декабря 2025 г., 21:37
 */


#include <xc.h>
#include "../__OD/OD.h"
#include "../_can/CAN.h"
#include "../__CO/srvCTL.h"

uint8_t DATAEE_ReadByte(uint8_t epp_adr);
void DATAEE_WriteByte(uint8_t epp_adr, uint8_t epp_data);
//extern void(* const funcLight[8])(uint8_t);
void srv_callBack(uint16_t *oldVal, uint16_t newVal);
void srvSetFlag(uint8_t mask);

extern epp_sdo_t epp_sdo;

// === SDO-сервер
extern uint8_t od_count;
extern const od_entry_t od[];
extern packCAN_t rxCAN;
extern packCAN_t txCAN;
extern packSDO_t packSDO;

void execCMD(uint8_t stateExec);

void initSendSDO(void)
{
	uint8_t cnt = 8;
	while (cnt) {
		cnt--;
		packSDO.raw[cnt] = rxCAN.data[cnt];
	}
}

void execSDO(void)
{
	const subind_t *ptr_subind;
	//будем формировать обрабатывать приняятый запрос и формировать ответ
	// === Найти объект и субиндекс
	initSendSDO();
	srvSetFlag(SYS_ANS_SDO); //установить флаг
	packSDO.cmd_spec = 0;
	packSDO.reqeue_type = ERR_RESP;
	for (uint8_t i = 0; i < od_count; i++) {
		if (od[i].index == rxCAN.packSDO.index_sdo) {
			ptr_subind = od[i].subs;
			for (uint8_t s = 0; s < od[i].sub_count; i++) {
				if (ptr_subind->num == rxCAN.packSDO.subindex_sdo) {
					packSDO.index_sdo = rxCAN.packSDO.index_sdo;
					packSDO.subindex_sdo = rxCAN.packSDO.subindex_sdo;
					goto index_OK;
				}
				ptr_subind++;
			}
			packSDO.error_code = ERR_SUBINDEX;
			return;
		}
	}
	packSDO.error_code = ERR_INDEX;
	return;
index_OK:
	// === Upload Request (чтение)

	if (rxCAN.packSDO.reqeue_type == UPL_REQU) { // 0b(010)0 0000)
		{
			if (ptr_subind->r_o != 1) {
				packSDO.error_code = ERR_RO_FAULT;
				return;
			}

			switch (ptr_subind->sdt) {
			case INTEGER8:
				packSDO.val08t[0] = *(uint8_t*) ptr_subind->value;
				packSDO.len_value = 1;
				break;
			case INTEGER16:
			{
				uint16_t val = *(uint16_t*) ptr_subind->value;
				packSDO.val16t[0] = val;
				packSDO.len_value = 2;
				break;
			}
			case INTEGER32:
			{
				uint32_t val = *(uint32_t*) ptr_subind->value;
				packSDO.val32t[0] = val;
				packSDO.len_value = 4;
				break;
			}
			case EPP_RW16:
			{
				//if (rxCAN.packSDO.len_value == 2) {
				//if (rxCAN.packSDO.val08t[1] == epp_sdo.address) { //addr - hi
				epp_sdo.value = DATAEE_ReadByte(epp_sdo.address); // читаем данные
				//*(uint8_t*) ptr_subind->value = 
				packSDO.val16t[0] = epp_sdo.adr_val;
				packSDO.len_value = 2;
				//return;
				//	}
				//}
				break;
			}
			default:
			{
				packSDO.error_code = ERR_UNKNOW_TYPE;
				return;
			}
			}
			packSDO.reqeue_type = UPL_RESP; // 0b(011)n nnnn
			return;
		}
	}
	// === Download Request (запись) 0b110nnnn

	if (rxCAN.packSDO.reqeue_type == DNL_REQU) // 0b110n nnnn
	{
		if (ptr_subind->w_o == 0) {
			packSDO.error_code = ERR_WR_FAULT;
			return;
		}
		packSDO.reqeue_type = DNL_RESP; // 0b1110 0000
		switch (ptr_subind->sdt) {
		case INTEGER8:
		{
			if (rxCAN.packSDO.len_value == 1) {
				*(uint8_t*) ptr_subind->value = rxCAN.packSDO.val08t[0];

				return;
			}
			break;
		}

		case INTEGER16:
		{
			if (rxCAN.packSDO.len_value == 2) {

				srv_callBack((uint16_t*) ptr_subind->value, rxCAN.packSDO.val16t[0]);

				//			*(uint16_t*) ptr_subind->value = rxCAN.packSDO.val16t[0];
				return;
			}
			break;
		}

		case INTEGER32:
		{
			if (rxCAN.packSDO.len_value == 4) {
				*(uint32_t*) ptr_subind->value = rxCAN.packSDO.val32t[0];
				return;
			}
			break;
		}
		case EPP_RW16:
		{
			if (rxCAN.packSDO.len_value == 2) {
				if (rxCAN.packSDO.val08t[1] == epp_sdo.address) { //addr - hi
					//*(uint16_t*) ptr_subind->value = rxCAN.packSDO.val08t[0]; //val -lo
					DATAEE_WriteByte(rxCAN.packSDO.val08t[1], rxCAN.packSDO.val08t[0]);
					return;
				}
			}
			break;
		}
		case FUN_EXEC:
		{
			if (rxCAN.packSDO.len_value == 2) {
				uint8_t stateExec;
				stateExec = (uint8_t) ((rxCAN.packSDO.val08t[1] & 0x07) << 3);
				stateExec = stateExec | (rxCAN.packSDO.val08t[0] & 0x07);
				execCMD(stateExec);
				return;
			}
			break;
		}

		default:
		{
			packSDO.error_code = ERR_UNKNOW_TYPE;
			return;
		}

		}
		packSDO.error_code = ERR_WRONG_TYPE;
		packSDO.cmd_spec = ERR_RESP;
		return;
	}
	//сбросить флаг
}
