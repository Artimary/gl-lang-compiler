// model.h
#pragma once
#include "cfg.h"

typedef struct TypeRefN TypeRefN;
typedef struct FuncSignatureArg FuncSignatureArg;
typedef struct FuncSignatureN FuncSignatureN;
typedef struct FuncDefN FuncDefN;
typedef struct ProgramUnit ProgramUnit;
typedef struct InputFile InputFile;

struct TypeRefN {
    char* name;
};

struct FuncSignatureArg {
    char* name;
    TypeRefN* type;
};

struct FuncSignatureN {
    TypeRefN returnType;
    FuncSignatureArg* args;
    int argsCount;
};

struct FuncDefN {
    char* name;
    FuncSignatureN* signature;
    CfgNode* cfg;
};

struct ProgramUnit {
    FuncDefN* funcs;
    int funcsCount;
    InputFile* inputFiles;  // ��������� ������������� ������
    int inputFilesCount;    // ���������� ������
    char* sourceFileName;   // ��� ��������� �����
    struct {
        int errors_count;
        char** error_messages;
    } errors;
};

struct InputFile {
    char* fileName;
    AstNode* ast;
};

// ������� ��� �������� ������ ���������
ProgramUnit* createModule(char* sourceFileName);

// ������� ��� ���������� ������� � ������
void addFunctionToModule(ProgramUnit* module, FuncDefN* func);

// ������� ��� ���������� ������ � ������
void addErrorToModule(ProgramUnit* module, char* errorMessage);

// ������� ��� ���������� ����� � ��������� ������������� ������
void addInputFileToModule(ProgramUnit* module, char* fileName, AstNode* ast);

// ������� ��� ������������ ������, ���������� ��� ������
void freeProgramUnit(ProgramUnit* programUnit);

extern InputFile* InputFiles;