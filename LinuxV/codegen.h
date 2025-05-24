#pragma once
#include "model.h"
#include <stdint.h>

// Структура для представления инструкции
typedef struct {
    char* mnemonic;      // Название операции (например, "ADD", "LDR")
    char* operands[3];   // Операнды (например, ["X0", "X1", "X2"])
    uint64_t address;    // Адрес в памяти
    uint8_t size;        // Размер инструкции в байтах (всегда 4 для AArch64)
} Instruction;

typedef enum {
    DT_WORD,    // 4-байтовое целое число
    DT_QUAD,    // 8-байтовое целое число
    DT_ASCIZ,   // Строка с нулевым окончанием
    DT_SPACE    // Неинициализированное пространство
} DataType;

// Структура для представления элемента данных
typedef struct {
    union {
        int64_t int_value;   // Литерал (целое число)
        char* str_value;     // Литерал (строка)
    } value;             // Значение элемента данных
    uint32_t size;       // Размер в байтах
    uint64_t address;    // Адрес в памяти
    DataType dtype;
    char* name;
} DataElement;

typedef struct {
    char* name;
    Instruction* instructions;
    int instr_count;
    DataElement* locals;
    int locals_count;
} FunctionCode;

typedef struct {
    FunctionCode* functions;
    int func_count;
    DataElement* globals;
    int global_count;
} ProgramImage;

// Прототипы функций
ProgramImage* generateProgramImage(ProgramUnit* unit);
void freeProgramImage(ProgramImage* img);
FunctionCode generateFunctionCode(FuncDefN* func, ProgramImage* img);
const char* processOpNode(OpNode* op, FunctionCode* fc, uintptr_t* address, int* reg_counter);
