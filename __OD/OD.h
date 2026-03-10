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
#define WO  2
#define RW  3

typedef union {

    struct {
        uint8_t value; //lo(в пакете) int8_t [0]]
        uint8_t address; //hi (в пакете) int8_t [1]
    };
    uint16_t adr_val;
} epp_sdo_t;

// Субиндекс

typedef struct {
    uint8_t num; // номер субиндекса

    union {
        uint8_t type; // свойства субиндекса

        struct {
            uint8_t sdt : 4; // тип данных
            uint8_t raw : 2; // резерв
            uint8_t r_o : 1; // только чтение
            uint8_t w_o : 1; // только запись
        };
    };
    void *value; // адрес значения субиндекса
    //    void(* execSDO)(uint16_t var);
} subind_t;

// Словарь объектов

typedef struct {
    uint16_t index; // номер индекса
    uint8_t sub_count; // количество субиндексов
    const subind_t *subs; // адрес массива субиндексов
} od_entry_t;
#endif