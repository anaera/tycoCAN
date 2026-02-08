/*
 * File:   updater.c
 * Author: Chiper
 *
 * Created on 6 марта 2025 г., 15:02
 */
#include "../_pins/pins.h"
#include "../_epp/epp.h"
#include "../__CO/CO.h"

void initPins(void);
void writePin(state_pin_t statePin);
uint8_t readPin(uint8_t numPin);
void updatePins(void);
void storeActivPin(uint8_t statePinVal);
//---ext
void scanTablePDO(uint8_t node, uint8_t activePin);
void checkPress(state_pin_t statePin);
//---asm
void clrTris(uint8_t reg_pin);
void setPin(uint8_t reg_pin);
void clrPin(uint8_t reg_pin);
uint8_t getVal(uint8_t reg_pin);


extern __eeprom ee_t _ee;
extern uint8_t state_node;

ctl_pin_t ctl_pin[8];
packPDO_t packPDO = {.length = 0, .raw =
	{0, 0, 0, 0, 0, 0, 0, 0}};

void initPins(void)
{
	state_pin_t statePin;
	for (uint8_t number = 0; number < MAX_PINS; number++) {
		ctl_pin[number].ctl = _ee.ctl_pin[number].ctl; //копируем  ROM в RAM
		if (ctl_pin[number].dir) { // настраиваем порты на ввод/вывод
			//setTris(ctl_pin[statePin.number].ctl); //инициализируем направление IN
		} else {
			// Отослано не будет т.к. state_node == 0
			//			if (ctl_pin[number].val) statePin.state = _UP; //начальное значение
			//else statePin.state = _DN;
			if (ctl_pin[number].act) { //если регистр
				clrTris(ctl_pin[number].ctl); // направление OUT
				if (ctl_pin[number].val) {
					setPin(ctl_pin[number].ctl);
				} else {
					clrPin(ctl_pin[number].ctl);
				}
			}
		}
	}
}

void storeActivPin(uint8_t statePinVal)
{
	scanTablePDO(CAN_NODE, statePinVal);
	if (state_node != NMT_STATE_OPER) return;
	if (packPDO.length < MAX_PINS) {
		packPDO.raw[packPDO.length] = statePinVal;
		packPDO.length++;
	}
}

uint8_t readPin(uint8_t numPin)
{
	return ctl_pin[numPin].val;
}

void writePin(state_pin_t statePin)
{
	uint8_t val = statePin.set.val;
	uint8_t num = statePin.number;
	if (statePin.set.act == (uint8_t) 0x01) { // если 6 бит статуса = 1 (это длина нажатия)
		return; // ничего не пишем
	}
	if (ctl_pin[num].dir) { //в биты чтения не пишем ничего
		return;
	}
	if (ctl_pin[num].act) { //если не пин, то ничего не переносим в регистр
		if (ctl_pin[num].inv) val = val ^ 1; //инвертируем для регистра
		if (val) {
			setPin(ctl_pin[num].ctl);
		} else {
			clrPin(ctl_pin[num].ctl);
		}
	}
	storeActivPin(statePin.total); // отсылаем (здесь далее будет анализ предидущего  
	ctl_pin[num].val = val; // значения ctl_pin[num].val поэтому только здесь изменяем!!!
}

void updatePins(void)
{
	uint8_t val;
	uint8_t num;
	state_pin_t statePin;

	for (num = 0; num < MAX_PINS; num++) {
		if (ctl_pin[num].act) { // читаем (извлекаем) только порты
			val = getVal(ctl_pin[num].ctl);
			if (ctl_pin[num].inv) val = val ^ 1; //если инверсия
			if ((val ^ ctl_pin[num].val) != 0) { // изменилось состояние
				statePin.state = val ? _UP : _DN;
				statePin.number = num;
				checkPress(statePin);
				ctl_pin[num].val = val;
			}
		}
	}
}





