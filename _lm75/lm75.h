
#ifndef LM75_H
#define	LM75_H

#include <xc.h> // include processor files - each processor file is guarded.  
#define TEMP_MAX 4

typedef struct {
    uint8_t lsbTemp;    //меняем выдачу на младшим байтом вперед !!!
    uint8_t msbTemp;

} msbLsb_t;

typedef union {
    msbLsb_t setTemp[TEMP_MAX]; // LM75 выдает temp Старшим байтом вперед
    uint8_t packTemp[2*TEMP_MAX]; // меняем на младшим впкркд для int16
    int16_t temp16t[TEMP_MAX];
} temp_t;

#endif	/* LM75_H */

