//#include "constant.h"
//#include "defined.h"
#include "../_epp/epp.h"
#include "../_pins/pins.h"
#include "../__CO/CO.h"

#define MAX_EXEC 8

#define DOOR_PIN 0
#define MOVE_PIN 1
#define BOTTON_PIN 2
#define LIGHT_PIN2 3
#define LIGHT_PIN 4
#define MANUAL_PIN 5

enum {
	_dnSetPinVal = 0,
	_upSetPinVal,
	_swSetPinVal,
	_upSetPinTimer,
	_quickPress,
	_longPress,
	_upStartLight,
	_pickUP,
} cmdAct;

void dnSetPinVal(uint8_t numPin);
void upSetPinVal(uint8_t numPin);
void swSetPinVal(uint8_t numPin);
void upSetPinTimer(uint8_t numPin);
void longPress(uint8_t numPin);
void quickPress(uint8_t numPin);
void upStartLight(uint8_t numPin);
void pickUP(uint8_t numPin);
void storeActivExec(uint8_t stateExec);
void execCMD(uint8_t stateExec);
void scanPackExec(void);
void scanTablePDO(uint8_t node, uint8_t activePin);

//---ext
void writePin(state_pin_t statePin);
uint8_t readPin(uint8_t numPin);
void setTimers(state_pin_t statePin);
void clrTimers(uint8_t numPin);


packPDO_t packExec;

extern __eeprom ee_t _ee;
extern ctl_pin_t ctl_pin[8];
extern uint8_t state_node;
extern uint8_t node_owner;
extern packPDO_t packPDO;

// void(* makeAction[4])(uint8_t numPin) = {dnSetPinVal, upSetPinVal, swSetPinVal, upSetPinTimer,
//, longPressFun, quickPress, upStartLight, pickUP};
//makeAction[tab.entry.action](tab.entPinNum));

void pickUP(uint8_t numPin)
{

}
//установить пин в единицу

void upSetPinVal(uint8_t numPin)
{
	numPin |= 0x80;
	writePin((state_pin_t) numPin);
	/*state_pin_t statePin;
	statePin.state = _UP;
	statePin.number = numPin;
	writePin(statePin);*/
}
//установить пин в ноль

void dnSetPinVal(uint8_t numPin)
{
	writePin((state_pin_t) numPin);
	/*state_pin_t statePin;
	statePin.state = _DN;
	statePin.number = numPin;
	writePin(statePin);*/
}
//переключить пин (инверсия)

void swSetPinVal(uint8_t numPin)
{
	if (readPin(numPin)) {
		writePin((state_pin_t) numPin);
	} else {
		numPin |= 0x80;
		writePin((state_pin_t) numPin);
	}
	/*state_pin_t statePin;
	if (readPin(numPin)) {
		statePin.state = _DN;
	} else {
		statePin.state = _UP;
	}
	statePin.number = numPin;
	writePin(statePin);*/
}
//включить пин и запустить его таймер

void upSetPinTimer(uint8_t numPin)
{
	upSetPinVal(numPin);
	setTimers((state_pin_t) numPin);

	/*state_pin_t statePin;
	upSetPinVal(numPin);
	statePin.state = _DN; //новое состояние после окончания таймера
	setTimers(statePin);*/
}

//Если AUTO то сбросить таймеp,
//иначе установить таймер и вкл свет ?? где это??
// Переключить режим

void longPress(uint8_t numPin)
{
	if (!readPin(MANUAL_PIN)) {
		clrTimers(numPin);
	} else {
		upSetPinTimer(numPin);
	}
	swSetPinVal(MANUAL_PIN);
}
// Если AUTO, включитьть свет и запустить таймер
//Иначе переключить SWAP свет

void quickPress(uint8_t numPin)
{
	if (readPin(MANUAL_PIN)) {
		upSetPinTimer(numPin);
		return;
	}
	swSetPinVal(numPin);
}

// Если AUTO, включитьть свет и запустить таймер

void upStartLight(uint8_t numPin)
{
	if (readPin(MANUAL_PIN)) {
		upSetPinTimer(numPin);
	}
}

void storeActivExec(uint8_t stateExec)
{
	if (packExec.length < MAX_EXEC) {
		packExec.raw[packExec.length] = stateExec;
		packExec.length++;
	}
}
//void(* const funcLight[8])(uint8_t) = {dnSetPinVal, upSetPinVal, swSetPinVal, upSetPinTimer, quickPress, longPress//, upStartLight, pickUP};
/*
void execRemote(uint8_t stateExec)
{
	tab_element_t tab;
	tab.element = stateExec;
	uint8_t numPin = tab.entry.actPinNum;
	uint8_t cmdAct = tab.entry.action;
	funcLight[cmdAct](numPin);
}
*/
void execCMD(uint8_t stateExec)
{
	tab_element_t tab;
	tab.element = stateExec;
	uint8_t numPin = tab.entry.actPinNum;
	uint8_t cmdAct = tab.entry.action;
		switch (cmdAct) {
	case _dnSetPinVal:
		dnSetPinVal(numPin);
		break;
	case _upSetPinVal:
		upSetPinVal(numPin);
		break;
	case _swSetPinVal:
		swSetPinVal(numPin);
		break;
	case _upSetPinTimer:
		upSetPinTimer(numPin);
		break;
	case _quickPress:
		quickPress(numPin);
		break;
	case _longPress:
		longPress(numPin);
		break;
	case _upStartLight:
		//quickPress(numPin);
		upStartLight(numPin);
		break;
	case _pickUP:
		pickUP(numPin);
		break;

	}
}

void scanPackExec(void)
{
	for (uint8_t cnt = 0; cnt < packExec.length; cnt++) {
		execCMD(packExec.raw[cnt]);
	}
	packExec.length = 0;
}

tab_element_t __eeprom *ptrEErom;

uint8_t getElement(tab_element_t __eeprom *ptrEErom)
{
	//return _ee.table[(*ptr)++].element;
	//return(*ptrEErom).element;
	return ptrEErom->element;
}

void scanTablePDO(uint8_t node, uint8_t activePin)
{
	state_pin_t statePin;
	uint8_t lenSec;
	tab_element_t tab;
	ptrEErom = &_ee.table[0]; // начало таблицы
	statePin.total = activePin;
	tab.element = getElement(ptrEErom);
	ptrEErom++; //читаем номер node

	while (tab.element != 0) {
		if (tab.element == node) {
			//			ptrEErom++;
			tab.element = getElement(ptrEErom);
			ptrEErom++; // читаем заголовок pin секции	
			lenSec = tab.lenEntSec; // длина текущей секции	
			while (lenSec > 0) // пока не кончится секция пинов
			{
				if (tab.entPinNum == statePin.number) {
					while (lenSec > 0) {
						tab.element = getElement(ptrEErom);
						ptrEErom++; //читаем действие
						if (tab.entry.stimulus == statePin.state) {
							//writeBuf_CMD(tab.element);
							//execCMD(tab.entry.actPinNum, tab.entry.action);
							storeActivExec(tab.element);
						}
						lenSec--;
					}
				} else {
					ptrEErom += lenSec; // пропускаем текущую секцию
				}
				tab.element = getElement(ptrEErom);
				ptrEErom++; // читаем заголовок pin секции
				lenSec = tab.lenEntSec; // длина следующей секции	
			}
			tab.element = getElement(ptrEErom);
			ptrEErom++; // читаем заголовок node секции		
		} else {
			// Пропускаем всю секцию пинов для текущего узла
			do {
				tab.element = getElement(ptrEErom);
				ptrEErom++; // читаем заголовок pin секции
				lenSec = tab.lenEntSec; // длина следующей секции
				ptrEErom += lenSec; // пропускаем секцию
			} while (lenSec > 0); // пока не кончится секция пинов
			tab.element = getElement(ptrEErom);
			ptrEErom++; // читаем заголовок pin секции
		}
	}
}
/*
 номер act node
 размер секции | номер act pin
 act state | local exec | local pin
 act state | local exec | local pin
 ......
 act state | local exec | local pin
  размер секции | номер act pin
 act state | local exec | local pin
 act state | local exec | local pin
 ......
 act state | local exec | local pin
 0
 номер act node
 размер секции | номер act pin
 act state | local exec | local pin
 act state | local exec | local pin
 ......
 act state | local exec | local pin
  размер секции | номер act pin
 act state | local exec | local pin
 act state | local exec | local pin
 ......
 act state | local exec | local pin
 0
 0  
 
 
 
 */