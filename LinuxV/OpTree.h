#pragma once
#include "treeStructure.h"

typedef struct OpNode OpNode;

typedef enum OpNodeKinds {
    OP_UNKNOWN = 0,     // �����������/�������������� ��������
    OP_READ,            // ������ �������� ���������� (��������, `x`)
    OP_WRITE,           // ������������ (��������, `x = 5`)
    OP_READ_INDEX,      // ������ �������� ������� (��������, `arr[i]`)
    OP_WRITE_INDEX,     // ������ � ������� ������� (��������, `arr[i] = 10`)

    // �������������� ��������
    OP_SUM,             // �������� (`a + b`)
    OP_SUB,             // ��������� (`a - b`)
    OP_NEG,             // ������� ����� (`-a`)
    OP_REM,             // ������� �� ������� (`a % b`)
    OP_DIV,             // ������� (`a / b`)
    OP_MUL,             // ��������� (`a * b`)

    // �������� ���������
    OP_CMP_GT,          // ������ (`a > b`)
    OP_CMP_GT_EQ,       // ������ ��� ����� (`a >= b`)
    OP_CMP_EQ,          // ����� (`a == b`)
    OP_CMP_NEQ,         // �� ����� (`a != b`)

    // ������� ��������
    OP_BITS_OR,         // ��������� ��� (`a | b`)
    OP_BITS_AND,        // ��������� � (`a & b`)
    OP_BITS_XOR,        // ��������� ����������� ��� (`a ^ b`)
    OP_BITS_INV,        // ��������� �������� (`~a`)
    OP_BITS_SHL,        // ����� ����� (`a << b`)
    OP_BITS_SHR,        // ����� ������ (`a >> b`)

    // ���������� ��������
    OP_BOOL_AND,        // ���������� � (`a && b`)
    OP_BOOL_XOR,        // ���������� ����������� ��� (�������������, ������ `a ^^ b`)
    OP_BOOL_NOT,        // ���������� �� (`!a`)
    OP_BOOL_OR,         // ���������� ��� (`a || b`)

    // ������� � ������
    OP_CALL,            // ����� ������� (`func()`)
    OP_PLACE,           // ����� � ������ (��������, ���������� ��� ������)

    // ���������
    OP_CONST_INT,       // ������������� ��������� (`5`, `-10`)
    OP_CONST_FLOAT,     // ����� � ��������� ������ (`3.14`, `-0.5`)
    OP_CONST_STRING,    // ��������� ��������� (`"hello"`)

    // ���������� �������
    OP_RETURN,          // ������� �� ������� (`return x`)

    // ��������� ��������� ������������
    OP_ADD_ASSIGN,      // ������������ �� ��������� (`a += b`)
    OP_DIV_ASSIGN,      // ������������ � �������� (`a /= b`)
    OP_CMP_LT_EQ,       // ������ ��� ����� (`a <= b`)
    OP_INC,             // ��������� (`a++`, `++a`)
    OP_DEC,             // ��������� (`a--`, `--a`)
    OP_SUB_ASSIGN,      // ������������ � ���������� (`a -= b`)
    OP_MUL_ASSIGN,      // ������������ � ���������� (`a *= b`)
    OP_REM_ASSIGN,      // ������������ � �������� (`a %= b`)
    OP_CMP_LT           // ������ (`a < b`)
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