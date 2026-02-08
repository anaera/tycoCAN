
//#include "defined.h"
#include "../_epp/epp.h"
#include "../_pins/pins.h"

#define PTR_MAX QPR_MAX
#define PTR_MIN 0

void initPressQueue(void);
void checkPress(state_pin_t statePin);
void dispatchPress(void);
void setTimers(state_pin_t statePin);
void moveTrailTimer(uint8_t ptr);
void clrTimers(uint8_t numPin);
void dispatchTimer(void);
//ext
void storeActivPin(uint8_t statePinVal);
void writePin(state_pin_t statePin);

extern __eeprom ee_t _ee;
queue_t qpr[QPR_MAX];
uint8_t maxPress;
uint8_t maxTimer;
//--------------------------------------------------------


void initPressQueue(void)
{
	maxPress = PTR_MIN;
	maxTimer = PTR_MAX;

	for (uint8_t cnt = 0; cnt < QPR_MAX; cnt++) {
		qpr[cnt].stQueue.total = 0; //очищаем 
		qpr[cnt].tmQueue = 0;
	}
}

void checkPress(state_pin_t statePin)
{
	uint8_t i;

	for (i = 0; i < maxPress; ++i) {
		if (qpr[i].tmQueue != 0) { // Элемент занят (время <> 0), идёт счёт
			if (qpr[i].stQueue.number == statePin.number) { // Занят этим номером
				if (qpr[i].stQueue.state == _DN || qpr[i].stQueue.state == _UP) {
					// Обработка фазы дребезга !! доработать !! <-------------------
					qpr[i].tmQueue = _ee.timePress.fixedTime; // Перезапускаем таймер
					return;
				}
			}
		}
	}

	// Если нет элемента с этим номером 
	// или для этого номера идёт счёт длительности состояния _DN
	// Заводим новый элемент и начинаем следующий цикл устранения дребезга, 
	for (i = 0; i < maxPress; ++i) {
		if (qpr[i].tmQueue == 0) { // Элемент свободен (время == 0)
			qpr[i].stQueue.total = statePin.total; //Занимаем
			qpr[i].tmQueue = _ee.timePress.fixedTime;
			return;
		}
	}
	// не свободного элементп до maxPress
	if (maxPress < maxTimer) {
		qpr[maxPress].stQueue.total = statePin.total;
		qpr[maxPress].tmQueue = _ee.timePress.fixedTime;
		maxPress++;
	}
	// нет свободных элементов
}

void setTimers(state_pin_t statePin)
{
	uint8_t i;
	// Есть элемент с этим номером 
	// Перезапускаем таймер
	for (i = maxTimer; i < QPR_MAX; i++) {
		if (qpr[i].tmQueue != 0) { // Элемент занят (время <> 0), идёт счёт
			if (qpr[i].stQueue.number == statePin.number) { // Занят этим номером
				qpr[i].tmQueue = _ee.timePress.timeTimers; // Перезапускаем таймер
				return;
			}
		}
	}

	// Нет элемента с этим номером 
	// Заводим новый элемент и начинаем следующий цикл устранения дребезга, 
	for (i = maxTimer; i < QPR_MAX; i++) {
		if (qpr[i].tmQueue == 0) { // Элемент свободен (время == 0)
			qpr[i].stQueue.total = statePin.total; //Занимаем
			qpr[i].tmQueue = _ee.timePress.timeTimers;
			return;
		}
	}
	// не свободного элементп до maxPress
	if (maxPress < maxTimer) //можно увеличить кол-во таймеров
	{
		maxTimer--; //добавляем в список
		qpr[maxTimer].stQueue.total = statePin.total;
		qpr[maxTimer].tmQueue = _ee.timePress.timeTimers;

	}
	// нет свободных элементов, не заводим таймер	

}

void moveTrailTimer(uint8_t ptr)
{
	qpr[ptr].stQueue.total = 0; // Очищаем
	qpr[ptr].tmQueue = 0; // Завершаем
	if (ptr != maxTimer) return;
	while ((maxTimer < QPR_MAX)&(qpr[ptr].tmQueue == 0)) {
		ptr++;
		maxTimer++;
	}
}

void clrTimers(uint8_t numPin)
{
	uint8_t i;
	for (i = maxTimer; i < QPR_MAX; i++) {
		if (qpr[i].stQueue.number == numPin) {
			moveTrailTimer(i); //придвигаем границу к ближайшему занятому
		}
	}
}

void dispatchTimer(void)
{
	uint8_t i;
	state_pin_t statePin;

	for (i = maxTimer; i < QPR_MAX; i++) {

		if (qpr[i].tmQueue) { // Время <> 0
			--qpr[i].tmQueue;

			if (qpr[i].tmQueue == 0) { // Закончилось
				statePin.total = qpr[i].stQueue.total;
				writePin(statePin); // установливаем новое состояние и !!отправляем
//				storeActivPin(qpr[i].stQueue.total); // Отправляем 
				moveTrailTimer(i); //придвигаем границу к ближайшему занятому
			}
		}
	}
}

void moveTrailPress(uint8_t ptr)
{
	qpr[ptr].stQueue.total = 0; // Очищаем
	qpr[ptr].tmQueue = 0; // Завершаем
	if (ptr != maxPress - 1) return;
	while ((maxPress > 0)&(qpr[ptr].tmQueue == 0)) {
		ptr--;
		maxPress--;
	}
}

void dispatchPress(void)
{
	uint8_t i, j;

	for (i = 0; i < maxPress; ++i) { //обходим пины]
		if (qpr[i].tmQueue) { // Время <> 0
			--qpr[i].tmQueue; //еще не ноль, уменьшаем

			if (qpr[i].tmQueue == 0) { // Закончилось
				switch (qpr[i].stQueue.state) { //в каком состоянии пин?
				case _UP: // Окончилось ожидание _UP
					for (j = 0; j < maxPress; ++j) {
						if (qpr[j].tmQueue != 0) { //не пустая
							if (qpr[j].stQueue.number == qpr[i].stQueue.number) { //равна
								if (i != j) { //не сама же
									//&&(qpr[j].stQueue.set.act == 0x01)) { // Это PRESS?
									storeActivPin(qpr[j].stQueue.total); // Отправляем PRESS
									moveTrailPress(j);
								}
							}
						}
					}
					storeActivPin(qpr[i].stQueue.total); // Отправляем _UP
					moveTrailPress(i);
					break;

				case _DN: // Окончилось ожидание _DN
					storeActivPin(qpr[i].stQueue.total); // Отправляем _DN
					qpr[i].stQueue.state = _QP; // Устанавливаем состояние quickPress
					qpr[i].tmQueue = _ee.timePress.quickTime; // И время quickPress
					break;

				case _QP: // Окончилось ожидание _QP
					qpr[i].stQueue.state = _LP; // Устанавливаем состояние _LP
					qpr[i].tmQueue = _ee.timePress.longTime; // И время longPress
					break;

				case _LP: // Окончилось ожидание _LP
					storeActivPin(qpr[i].stQueue.total); // Отправляем _LP
					qpr[i].tmQueue = _ee.timePress.repitTime; // Время repit и состояние _LP
					break;
				}
			}
		}
	}
}