#pragma once
#include "treeStructure.h"

typedef struct OpNode OpNode;

typedef enum OpNodeKinds {
    OP_UNKNOWN = 0,     // Неизвестная/нераспознанная операция
    OP_READ,            // Чтение значения переменной (например, `x`)
    OP_WRITE,           // Присваивание (например, `x = 5`)
    OP_READ_INDEX,      // Чтение элемента массива (например, `arr[i]`)
    OP_WRITE_INDEX,     // Запись в элемент массива (например, `arr[i] = 10`)

    // Арифметические операции
    OP_SUM,             // Сложение (`a + b`)
    OP_SUB,             // Вычитание (`a - b`)
    OP_NEG,             // Унарный минус (`-a`)
    OP_REM,             // Остаток от деления (`a % b`)
    OP_DIV,             // Деление (`a / b`)
    OP_MUL,             // Умножение (`a * b`)

    // Операции сравнения
    OP_CMP_GT,          // Больше (`a > b`)
    OP_CMP_GT_EQ,       // Больше или равно (`a >= b`)
    OP_CMP_EQ,          // Равно (`a == b`)
    OP_CMP_NEQ,         // Не равно (`a != b`)

    // Битовые операции
    OP_BITS_OR,         // Побитовое ИЛИ (`a | b`)
    OP_BITS_AND,        // Побитовое И (`a & b`)
    OP_BITS_XOR,        // Побитовое исключающее ИЛИ (`a ^ b`)
    OP_BITS_INV,        // Побитовая инверсия (`~a`)
    OP_BITS_SHL,        // Сдвиг влево (`a << b`)
    OP_BITS_SHR,        // Сдвиг вправо (`a >> b`)

    // Логические операции
    OP_BOOL_AND,        // Логическое И (`a && b`)
    OP_BOOL_XOR,        // Логическое исключающее ИЛИ (нестандартное, иногда `a ^^ b`)
    OP_BOOL_NOT,        // Логическое НЕ (`!a`)
    OP_BOOL_OR,         // Логическое ИЛИ (`a || b`)

    // Функции и память
    OP_CALL,            // Вызов функции (`func()`)
    OP_PLACE,           // Место в памяти (например, переменная для записи)

    // Константы
    OP_CONST_INT,       // Целочисленная константа (`5`, `-10`)
    OP_CONST_FLOAT,     // Число с плавающей точкой (`3.14`, `-0.5`)
    OP_CONST_STRING,    // Строковая константа (`"hello"`)

    // Управление потоком
    OP_RETURN,          // Возврат из функции (`return x`)

    // Составные операторы присваивания
    OP_ADD_ASSIGN,      // Присваивание со сложением (`a += b`)
    OP_DIV_ASSIGN,      // Присваивание с делением (`a /= b`)
    OP_CMP_LT_EQ,       // Меньше или равно (`a <= b`)
    OP_INC,             // Инкремент (`a++`, `++a`)
    OP_DEC,             // Декремент (`a--`, `--a`)
    OP_SUB_ASSIGN,      // Присваивание с вычитанием (`a -= b`)
    OP_MUL_ASSIGN,      // Присваивание с умножением (`a *= b`)
    OP_REM_ASSIGN,      // Присваивание с остатком (`a %= b`)
    OP_CMP_LT           // Меньше (`a < b`)
} OpNodeKinds;

struct OpNode {
    OpNodeKinds kind;
    OpNode** args;
    int argsCount;
    union {
        int integer;
        float real;
        char* string;
    } payload;
};

OpNode* buildOpTree(AstNode* astNode);
void freeOpTree(OpNode* node);
char* opTreeToString(OpNode* node);