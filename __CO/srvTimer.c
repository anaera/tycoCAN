/*
 * File:    timerService.c
 * Author:  Chiper (улучшено для PIC)
 * Target:  Microchip 8-bit MCUs (PIC16/PIC18)
 * Desc:    Управление софт-таймерами с минимальным overhead.
 *          Использует указатели для эффективной работы в циклах.
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "../__CO/srvCTL.h"



// Статический массив таймеров (в ОЗУ, минимальный доступ)
static timer_entry_t srvTimers[MAX_TIMERS] = { {0, NULL} };

// Счётчик активных таймеров (0..MAX_TIMERS)
static uint8_t timerCount = 0;

// Управление флагами, прототипы внутренних функций
uint16_t updFlagAndTime(service_t *srvSRV);
bool srvChkFlag(uint8_t mask);
void srvSetFlag(uint8_t mask);
/**
 * Устанавливает флаг сервиса и возвращает его период ожидания.
 * Вызывается при истечении таймера.
 * 
 * @param srv - указатель на структуру сервиса
 * @return период ожидания в тиках
 */
uint16_t updFlagAndTime(service_t *srvSRV)
{
	NOP();
	srvSetFlag(srvSRV->mask); //взводим флаг
	return srvSRV->waitTime; // возвращаем время из сервисва
}
/**
 * Регистрирует сервис в системе софт-таймеров.
 * 
 * @param srvSTRUCT - указатель на структуру сервиса
 * @return true, если регистрация успешна
 */
bool initTimerEntry(service_t *srvSTRUCT)
{
	// Проверка: место есть?
	if (timerCount >= MAX_TIMERS) {
		return false;  // Нет места
	}
	    if (srvSTRUCT->handle) {
        return false; // Повторная регистрация
    }
	// Инициализация элемента
	srvTimers[timerCount].coutTime = 0; // Сброс счётчика
	srvTimers[timerCount].srvStruct = srvSTRUCT;
	// Передаём хэндл обратно в сервис 
	srvSTRUCT->handle = timerCount;
	// Устанавливаем указатель на следующий элемент
	timerCount++;
	return true;
}

/*
 * Основной цикл обновления таймеров.
 * Должен вызываться регулярно (например, из прерывания каждые 1 мс, 10 мс или 100мс).
  * Использует указатель для быстрого доступа к элементам (эффективно на PIC).
 */
void checkTimers(void)
{
	timer_entry_t *pTimer = srvTimers; // Указатель на первый таймер
	uint8_t count = timerCount; // Локальная копия (оптимизация)

	// Обход всех зарегистрированных таймеров через указатель
	while (count--) {
		// Пропускаем неактивные таймеры
		if (pTimer->srvStruct != NULL) {
			if (pTimer->coutTime == 0) {
				// Таймер истёк: вызываем функцию действия
				// Она установит флаг и вернёт новый период
				pTimer->coutTime = updFlagAndTime(pTimer->srvStruct);
			} else {
				// Уменьшаем счётчик
				pTimer->coutTime--;
			}
		}
		// Переход к следующему таймеру (указательная арифметика — быстро)
		pTimer++;
	}
}
/**
 * Отключает таймер.
 * 
 * @param handle - идентификатор таймера (0..3)
 */
void clrTimerHandle(uint8_t handle)
{
	if (handle < MAX_TIMERS) {
		srvTimers[handle].srvStruct->handle =0; //удаление ссылки на таймер
		srvTimers[handle].srvStruct = NULL; // Деактивация таймера
	}
}
/**
 * Перезапускает таймер.
 * 
 * @param handle - идентификатор таймера
 */
void updTimerHandle(uint8_t handle)
{
	if (handle < MAX_TIMERS) {
		srvTimers[handle].coutTime = 0;
	}
}