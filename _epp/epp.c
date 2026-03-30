/*
 * File:   epp.c
 * Author: Chiper
 *
 * Created on 11 февраля 2025 г., 13:05
 */


#include "../_epp/epp.h"

//------------------------------------------ROM------------
__eeprom ee_t _ee = {
	//section canEngine
	CAN_SPEED,
	GAP_TIME,
	RCV_MASK, // bit_mask; маска бита выборки данных
	CRC_MASK, //_crc_poly; полином CRC

	//section canOpen
	CAN_NODE, //node 
	START_STATE,

	0x3C,
	0x00, // _beatTime 60 !младшим байтом вперед!
	0x1E,
	0x00, //_syncTime time 30 !младшим байтом вперед!
	0x32,
	0x00, //_tempTime 50
	0x0A,
	0x00, // _waitTime 10 !младшим байтом вперед!

	//section canApplication
	//ctl_pin_t[8]
	/*0xE3, //11100 011   RA3
	0xE4, //11100 100	RA4
	0xE5, //11100 101	RA5
	0xA0, //10100 000	RA0
	0xA1, //10100 001	RA1
	7, //00000 101	MR5
	7, //00000 110   MR6 
	7, //00000 111   MR7 */

	0xE0, //RA0 in move
	0xE1, //RA1 in door
	0xE2, //RA2 in botton
	0x94, //RA4 out L2
	0x95, //RA5 out L1
	0x8A, // RC2 out man
	0x8B, //RC3 out pwm
	0x07, //MR7 

	0xFF,	// by_event ctl_chk_t[8]
	0x38,   // by_timer
	0xF8,	// by_request
	0xE7, 0xE7, 0xE7, 0xE7, 0xE7, //ctl_chk_t[8]

	TIME_BOUNCE, //  timeTimers.fixedTime;
	TIME_QUICK, //	timeTimers.quickTime
	TIME_LONG, //	timeTimers.longTime
	TIME_REPIT, //	timeTimers.repitTime
	TIME_TIMERS_WAIT,
	//таблица управления светом
	CAN_NODE,
	
	
	/*0x10, 0x64, 0xEB,
	0x11, 0x34, 0xB4,
	0x12, 0xB4, 0x34,
	0x00, 0x00, 0x00,*/

	0x12, 0x64, 0xED,
	0x10, 0x34, 0xB4,
	0x11, 0x34, 0xB4,
	0x00, 0x00, 0x00,


	//section pinEngine
	/*
	0xE5, 0xE4, 0xE3, 0x90, 0x91, 0x9A, 0x40, 0x01,
	//inA5 inA4 inA3  outA0 outA1 outC2 outC3 dis00
	0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87,
	TIME_WAIT, TIME_QUICK, TIME_LONG, TIME_REPIT, TIMERS_TIME,
	_NODE, _SEC0, _SEC0_VAL0, _SEC0_VAL1, _SEC1, _SEC1_VAL0, _SEC2, _SEC2_VAL0, _END_SEC, _END_TAB,
		
	//нулевой пин 2 ячейки
	0x0C,      0x10,         0x03, 0x8B,
	//0          1             2     3    
	//первый пин 2 ячейки
				0x19,        0x0C, 0x84, 0xD5, 
	//            4            5     6    7
				0x00,
	//           8
	0x0E,       0x19,        0x0C, 0x84, 0xD5, 
	//9          A             B     C     D
				0x00,
	//           E
	0x0B,       0x19,        0x0C, 0x84, 0xD5, 
	//F          10             11   12   13
				0x00,
	//           14
	0x00
//    15*/
};

uint8_t getControlPin(uint8_t num)
{
	return(_ee.ctl_chk[num].check);
}
/*
uint8_t getStatePin(uint8_t num)
{
	return(_ee.ctl_pin[num].ctl);
}
 */