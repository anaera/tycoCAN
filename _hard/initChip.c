/*
 * File:   initChip.c
 * Author: Chiper
 *
 * Created on 9 февраля 2025 г., 18:33
 */
#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include <stdint.h>
#include "initChip.h"
//--------------------------------------------------------

void initWDT(void)
{
	//init_WDT

	WDTCON = 0b00010010; //512ms
	//        WDTCONbits.SWDTEN=1;
}

void initCPU(void)
{
	// SCS FOSC; SPLLEN disabled; IRCF 16MHz_HF; 
	OSCCON = 0x78;
	// TUN 0; 
	OSCTUNE = 0x00;
	// SBOREN disabled; 
	BORCON = 0x00;
}

void initPORTA(void)
{
	//init_PORTA

	PORTA = 0x00;
	LATA = 0x00;
	ANSELA = 0x00; // all analog to digital I/O    
	TRISA = 0x3F; //all pins input
	INTCON = 0x00; //сброс флага INTF

}

void initPORTC(void)
{
	//init_PORTC
	PORTC = 0x00;
	LATC = 0x00;
	ANSELC = 0x00; // all analog to digital I/O 
	TRISC = 0x3F; //all pins input
}
//---------------------------------------------------------------------
// Инициализация ECCP2
//---------------------------------------------------------------------

void initECCP2(void)
{
	// RC3 выставляем на выход
	TRISCbits.TRISC3 = 0;
	// PM single; DCB 0x0; CCPM P2A: active high; P2B: active high; 
	CCP2CON = 0x0C;
	// CTSEL PWM2timer6; 
	CCPTMRS0 = 0x08;
	// CCPASE operating; CCPAS disabled; PSSAC low; PSSBD low; 
	CCP2AS = 0x0;
	// PRSEN manual_restart; PDC 0x0; 
	PWM2CON = 0x0;
	// STRSYNC start_at_begin; STRB P2B_to_port; STRA P2A_to_CCP2M; 
	PSTR2CON = 0x01;
	// CCPRL 255; 
	CCPR2L = 125;
	// CCPRH 0x0; 
	CCPR2H = 0x0;
}


//---------------------------------------------------------------------
// Инициализация UART
//---------------------------------------------------------------------

void initUART(uint8_t num)
{
	//500, 250, 125, 62.5, 31.25, 15.1625, 15.1625, 15.1625
	const uint8_t tab_speed[8] = {7, 15, 31, 63, 127, 255, 255, 255};
	//const uint8_t len_time_bit = {2,  4,  8, 16,  32,  64,  64,  64} мкс;
	//const uint8_t len_cecle_rcv = 150; //микросекунд
	//gat_time = (10*len_time_bit+lencycle_rcv)/50
	//170, 190, 230, 310, 470, 790, 790, 790
	//  4,   4,   5,   6,  10,  16,  16,  16 *50
	//  2,   2,   3,   4,   5,   8,   8,   8 *100 
	//   ANSELHbits.ANS11 = 0;
	// настройка пинов
	TRISCbits.TRISC4 = 0; //TxD 
	TRISCbits.TRISC5 = 1; //RxD
	// сброс регистров
	TXSTA = 0;
	RCSTA = 0;
	BAUDCON = 0;
	// включение УАРТ
	TXSTAbits.TXEN = 1; //TX Включить TX передачу
	RCSTAbits.CREN = 1; //CREN Включить RX прием
	// настраиваем скорость обмена
	// SYNC=0 BRGH=1 BRG16=1   ->  Fosc/4*[N+1] 
	TXSTAbits.SYNC = 0; //TX Асинхронный режим SYNC
	TXSTAbits.BRGH = 1; //высокоскоростной BRGH
	BAUDCONbits.BRG16 = 1; //BRG16 16-битный генератор скорости
	SPBRGL = tab_speed[num]; //читаем делитель скорости
	SPBRGH = 0;
	// разрешаем прием
	RCSTAbits.SPEN = 1; //SPEN Включить УАРТ1 

	//Устанавливаем приориет прерывания
	//   IPR1bits.RC1IP = 1; //HI priority RxD
}
//----------------------------------------------------------------------

/*
void initTMR0(void)
{
	//init_TMR0
	// PSA assigned; PS 1:4; TMRSE Increment_hi_lo; mask the nWPUEN and INTEDG bits
	OPTION_REG = (uint8_t) ((OPTION_REG & 0xC0) | (0xD1 & 0x3F));
	// TMR0 206; 
	TMR0 = _TMR0; //0xCE см.ISR();
	//OSC = 16 000 000; тактирование Fosc/4 = 16000000/4 = 4 000 000
	//Прескалер 1:4, (Fosc/4 ) / 4 = 4 000 000/4 = 1 000 000
	//FF-CE = 31(49dec)
	//один тик 1 000 000 гц -> 1 мкс *49 = 49 (50)мкс (us)

}
//------------------------------------------------------------------------------

void initTMR1(void)
{
	//init_TMR1
	//T1GSS T1G_pin; TMR1GE disabled; T1GTM disabled; T1GPOL low; T1GGO done; T1GSPM disabled; 
	T1GCON = 0x00;
	CCP1CON = 0;
	////!!!!!!!!!!!!!!!!!!!!!!!!!! pic16F1824 разремить !!
	// CCP2CON = 0;
	//TMR1H 248; 
	TMR1H = _TMR1H; //0xF8 см.ISR();
	//TMR1L 48; 
	TMR1L = _TMR1L; //0x30 см.ISR();
	// T1CKPS 1:2; T1OSCEN disabled; nT1SYNC synchronize; TMR1CS FOSC/4; TMR1ON enabled; 
	T1CON = 0x11;
	T1CONbits.TMR1ON = 1; // TMR1 разрешен 

	//   T1CON = 0b11110001; // TMR1 разрешен
	//OSC = 16 000 000; тактирование Fosc/4 = 16000000/4 = 4 000 000
	//Прескалер 1:2, (Fosc/4 ) / 4 = 4 000 000/2= 2 000 000 (500ns)
	//FFFF-F830 = 7CF(1999dec)
	//один тик 500ns * 2000 = 1мс (ms)
	//Устанавливаем приориет прерывания   
	//   IPR1bits.TMR1IP = 1; //HI priority
}
//------------------------------------------------------------------------------




//timer 1ms 
 */

void initINT(void)
{
	//Разрешаем прерывание от Таймера 0
	//INTCONbits.TMR0IF = 0;
	//INTCONbits.TMR0IE = 1;
	//Разрешаем прерывание от Таймера 1
	//PIR1bits.TMR1IF = 0;
	//PIE1bits.TMR1IE = 1;
	//Разрешаем прерывание от Таймера 2
	//    PIR1bits.TMR2IF = 0;
	//    PIE1bits.TMR2IE = 1;

#ifdef INT_RA2
	//Разрешаем прерывание от INT0
	OPTION_REGbits.INTEDG = 0; //прерывание по падающему фронту
	INTCONbits.INTF = 0;
	INTCONbits.INTE = 0; //НЕ_Разрешаем прерывание от INT0
#else
	//Разрешаем прерывание от RA3 ао падающему фронту
	IOCANbits.IOCAN3 = 1; //спадающий фронт - ДА
	IOCAPbits.IOCAP3 = 0; //восходящий  - НЕТ

	IOCAFbits.IOCAF3 = 0; //сбрасываем флаг RA3
	INTCONbits.IOCIF = 0; //сбрасываем флаг прерывания
	INTCONbits.IOCIE = 0; //НЕ_Разрешаем прерывание от RA
#endif

	//Разрешаем прерывание от I2C
	PIR1bits.SSP1IF = 0;
	PIE1bits.SSP1IE = 1;
	//Разрешение прерывание от приемника УАРТ
	PIE1bits.RCIE = 1;
	//RCONbits.IPEN = 1; //разрешаем приоритет прерываний
	INTCONbits.PEIE = 1; //Разрешаем прерывания от переферии
	INTCONbits.GIE = 1; //разрешаетвысокоприоритетные прерывания
	//    INTCONbits.GIEL = 0; //разрешает низкоприоретееные прерывания    
}

void initChip(void)
{
	initCPU();
	initWDT();
	initPORTA();
	initPORTC();
	initECCP2();
	return;
}
