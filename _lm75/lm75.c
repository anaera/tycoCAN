/*
 * File:   lm75.c
 * Author: Chiper
 *
 * Created on 22 марта 2025 г., 15:12
 */


#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "../__CO/srvCTL.h"
#include "../_lm75/lm75.h"

void(* i2cStepExec)(void);
void i2cFinishExit(void);
void i2cIdleStep(void);
void i2cStopStep(void);
void i2cGetLSByteLM75(void);
void i2cGetMSByteLM75(void);
void i2cReadEnLM75(void);
void i2cSendAdrLM75(void);
void i2cStartLM75(void);

void i2cInit(void);
void i2cOpen(void);
void i2cClose(void);

void startLM75(void);
uint8_t getTempByte(uint8_t cnt);

volatile uint8_t cntTemp; //счетчик датчиков
uint8_t adrLM75[TEMP_MAX] = {0x48, 0x49, 0x4A, 0x4B}; //адреса датчиков

extern temp_t tempVal; //пакет температуры для отсылки
extern srv_t srv;
//------------------------------------------------------------------------------
//инициализация i2c 
//------------------------------------------------------------------------------

void i2cInit(void)
{
	TRISCbits.TRISC0 = 1; //SCL
	TRISCbits.TRISC1 = 1; //SDA
	SSP1STAT = 0x00;
	SSP1CON1 = 0x08;
	SSP1CON2 = 0x00;
	SSP1ADD = 0x27; //= 39; //100kgz=Fi2c = Fosc/(4*(SSP1ADD+1));
	// 16000000/(4*(39+1) = 100000
	SSP1CON1bits.SSPEN = 0;
}

void i2cOpen(void)
{
	i2cStepExec = i2cIdleStep; //set idlr step
	SSP1CON1bits.SSPEN = 1; //enable i2c
	PIR1bits.SSP1IF = 0; //clear irq
	PIE1bits.SSP1IE = 1; //enable irq
	cntTemp = 0;
}

void i2cClose(void)
{
	PIE1bits.SSP1IE = 0; // disable irq
	SSP1CON1bits.SSPEN = 0; //disable i2c
}


//------------------------------------------------------------------------------
//машина состояний i2c (чтение двух байт по адресу устройства)
//------------------------------------------------------------------------------

void i2cIdleStep(void)
{
	return;
}

void i2cExitStep(void)
{
	cntTemp++; //следующий датчик 42 мкс
	if (cntTemp < TEMP_MAX) {
		i2cStartLM75(); //следующий START
	} else {
		cntTemp = 0; //конец списка датчиков IDLE
		i2cStepExec = i2cIdleStep;
		srv.sysTMP_PDO = true; //флаг конца выбоки температуры из датчиков
	}
}

void i2cStopStep(void)
{
	i2cStepExec = i2cExitStep; //далее будет idle
	SSP1CON2bits.PEN = 1;

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
}

void i2cGetLSByteLM75(void)
{
	tempVal.setTemp[cntTemp].lsbTemp = SSP1BUF; // забираем данные из регистра
	i2cStepExec = i2cStopStep;
	SSP1STATbits.BF = 0;
	SSP1CON2bits.ACKDT = 1; //устанавливаем NACK
	SSP1CON2bits.ACKEN = 1; //шлем в шину

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>

}

void i2cAskLSByteLM75(void)
{
	i2cStepExec = i2cGetLSByteLM75;
	SSP1CON2bits.RCEN = 1; //запрос данных

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
}

void i2cGetMSByteLM75(void)
{
	tempVal.setTemp[cntTemp].msbTemp = SSP1BUF; // забираем данные из регистра
	i2cStepExec = i2cAskLSByteLM75;
	SSP1STATbits.BF = 0;
	SSP1CON2bits.ACKDT = 0; //устанавливаем ACK
	SSP1CON2bits.ACKEN = 1; //шлем в шину

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
	//после каждого ACKEN происходит прерывание.
	// Далее либо продолжаем  чтение RCEN либо конец STOP
}

void i2cReadEnLM75(void)
{
	if (SSP1CON2bits.ACKSTAT) { //адрес на шине подтвержден? //26 мкс
		i2cStepExec = i2cExitStep; //далее idle		
		SSP1CON2bits.PEN = 1; //шлем  STOP при ошибке
	} else {
		i2cStepExec = i2cGetMSByteLM75;
		SSP1CON2bits.RCEN = 1; //запрос данных
	}
	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
}

void i2cSendAdrLM75(void)
{
	i2cStepExec = i2cReadEnLM75;
	SSP1BUF = (uint8_t) (adrLM75[cntTemp] << 1) | 0x01; //адрес и флаг чтения в буфер на передачу

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
}
#pragma interrupt_level 1

void i2cStartLM75(void)
{
//	i2c.i2cAdr = (uint8_t) (adrLM75[cntTemp] << 1) | 0x01; //адрес и флаг чтения
	i2cStepExec = i2cSendAdrLM75;
	SSP1CON2bits.SEN = 1; //шлем в шину START

	//	PIR1bits.SSP1IF = 1; //-------------------------------------------->>
}

//------------------------------------------------------------------------------
//запуск цепочки чтения из нескольких устройств i2c 
// формирование пакета PDO1
//------------------------------------------------------------------------------

uint8_t getTempByte(uint8_t cnt)
{

	if (cnt < (TEMP_MAX + TEMP_MAX)) {
		return tempVal.packTemp[cnt];
	}
	return 0xFF;
}

void startLM75(void)
{
	cntTemp = 0;
	i2cStartLM75();
};
void initLM75(void){
	i2cStepExec = i2cIdleStep;
}