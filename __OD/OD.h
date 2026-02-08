#ifndef OBJDIR_H
#define OBJDIR_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

// Типы данных
#define BOOLEAN  0
#define INTEGER8  1
#define INTEGER16  2
#define INTEGER32  3
#define EPP_RW16  4
#define FUN_EXEC  5
// Режим доступа
#define CONST  0
#define RO  1
#define wO  2
#define RW  3


// Субиндекс

typedef struct {
    uint8_t num;

    union {
        uint8_t type;

        struct {
            uint8_t sdt : 4;
            uint8_t raw : 2;
            uint8_t r_o : 1;
            uint8_t w_o : 1;
        };
    };
    void *value;
    //    void(* execSDO)(uint16_t var);
} subind_t;

// Словарь объектов

typedef struct {
    uint16_t index;
    uint8_t sub_count;
    subind_t *subs;
} od_entry_t;
#endif