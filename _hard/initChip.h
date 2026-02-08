/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef INITCHIP_H
#define	INITCHIP_H
#include <xc.h> // include processor files - each processor file is guarded. 

//#define INT_RA2
#define PWM_TMR6_RC3_P1C

void initWDT(void);
void initCPU(void);
void initPORTA(void);
void initPORTC(void);
void initUART(uint8_t num);
void initTMR0(void);
void initTMR1(void);
void initINT(void);
void initChip(void);

#endif	/* INITCHIP_H */

