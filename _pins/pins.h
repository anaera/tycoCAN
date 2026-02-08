/*
 * File:   pins.h
 * Author: Chiper
 *
 * Created on January 13, 2026, 10:19 PM
 */


#include <xc.h>
#define QPR_MAX 8
#define MAX_PINS 8
//#define MASK_PRESS 1
#define _QP 1
#define _LP 3
//#define MASK_VALUE 2
#define _UP 2
#define _DN 0

typedef struct {
    uint8_t number : 6;
    uint8_t act : 1;
    uint8_t val : 1;
} set_t;

typedef union {
    uint8_t total;

    struct {
        uint8_t number : 6;
        uint8_t state : 2;
    };
    set_t set;
} state_pin_t;

typedef struct {
    state_pin_t stQueue;
    uint8_t tmQueue;
} queue_t;