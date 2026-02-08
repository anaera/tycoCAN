
//PAPA----------------------------------------------------------
/**
 * \file
 * \brief Обработчик прерываний
 * 
 * Таймер T0
 * формирование интервалапрерываний каждые 50мкс,
 * счетчик интервала 1мс,
 * формирование межпакетного интервала неактивности шины,
 * переход в режим передачи при неактивности шины.
 *
 * Приемник USART
 * прием байта в прерывании,
 * последующий вывод байта в режиме передачи.
 *
 * Контроль активности шины по прерыванию INT0
 * обнаружение падения уровня на шине (активности шины).
 *
 */
#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>
#include "../_hard/initChip.h"
#include "../_hard/TMRx.h"
#include "../_hard/canDrive.h"
#include "../_hard/debug.h"

//void(* i2cStepExec)(void);

static uint16_t appTime, sysTime;
extern tmr_t tmr;

inline void setStateReceive(void);
inline void packActEngine(void);
void(* i2cStepExec)(void);

void __interrupt() fullINT(void)
{
	if (INTCONbits.TMR0IF && INTCONbits.TMR0IE) { //триггер на 200мкс
		INTCONbits.TMR0IF = 0;
		tmr.gapFlag = 0;
		//RA1_SetLow();
		//RA0_Toggle();
		//RA0_SetLow();
		return;
	}
	if (PIR3bits.TMR4IF && PIE3bits.TMR4IE) { //1мс
		PIR3bits.TMR4IF = 0;
		NOP();
		TMR4 = _TMR4;
		sysTime++;
		tmr.sysFlag = 1;
		return;
	}
	if (PIR1bits.TMR2IF && PIE1bits.TMR2IE) { //10мс
		PIR1bits.TMR2IF = 0;
		NOP();
		TMR2 = _TMR2;
		tmr.cycFlag = 1;
		return;
	}
	if (PIR1bits.TMR1IF && PIE1bits.TMR1IE) { //100мс
		PIR1bits.TMR1IF = 0;
		TMR1H = _TMR1H;
		TMR1L = _TMR1L;
		NOP();
		appTime++;
		tmr.appFlag = 1;
		//		setTickFlag(); // взводим флаг 1мс
		return;
	}

#ifdef INT_RA2
	if (INTCONbits.INTE && INTCONbits.INTF) {
		INTCONbits.INTE = 0; //запрещаем прерывание от INT0	
		INTCONbits.INTF = 0; //сбрасываем флаг  
		setStateReceive(); //переход в режим приема
	}
#else
	if (INTCONbits.IOCIE && INTCONbits.IOCIF) {

		if (IOCAFbits.IOCAF3) {
			INTCONbits.IOCIE = 0;
			INTCONbits.IOCIF = 0;
			IOCAFbits.IOCAF3 = 0;
			setStateReceive(); //переход в режим приема
		}
	}
#endif
	if (PIE1bits.RCIE && PIR1bits.RCIF) {
		PIR1bits.RCIF = 0;
		packActEngine();
	}

	if (INTCONbits.PEIE == 1) {
		if (PIE2bits.BCL1IE == 1 && PIR2bits.BCL1IF == 1) {
			PIR2bits.BCL1IF = 0;
		}
		if (PIE1bits.SSP1IE == 1 && PIR1bits.SSP1IF == 1) {
			PIR1bits.SSP1IF = 0;
			i2cStepExec();
		}

	}

}



//----------------------------------------------------------------------

