#pragma once
#include "treeStructure.h"

typedef struct OpNode OpNode;

typedef enum OpNodeKinds {
    OP_UNKNOWN = 0,
    OP_READ,
    OP_WRITE,
    OP_READ_INDEX,
    OP_WRITE_INDEX,
    OP_SUM,
    OP_SUB,
    OP_NEG,
    OP_REM,
    OP_DIV,
    OP_MUL,
    OP_CMP_GT,
    OP_CMP_GT_EQ,
    OP_CMP_EQ,
    OP_BITS_OR,
    OP_BITS_AND,
    OP_BITS_XOR,
    OP_BITS_INV,
    OP_BITS_SHL,
    OP_BITS_SHR,
    OP_BOOL_AND,
    OP_BOOL_XOR,
    OP_BOOL_NOT,
    OP_BOOL_OR,
    OP_CALL,
    OP_PLACE,
    OP_CONST_INT,
    OP_CONST_FLOAT,
    OP_CONST_STRING,
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
