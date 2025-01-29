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
    InputFile* inputFiles;  // Коллекция анализируемых файлов
    int inputFilesCount;    // Количество файлов
    char* sourceFileName;   // Имя исходного файла
    struct {
        int errors_count;
        char** error_messages;
    } errors;
};

struct InputFile {
    char* fileName;
    AstNode* ast;
};

// Функция для создания модуля программы
ProgramUnit* createModule(char* sourceFileName);

// Функция для добавления функции в модуль
void addFunctionToModule(ProgramUnit* module, FuncDefN* func);

// Функция для добавления ошибки в модуль
void addErrorToModule(ProgramUnit* module, char* errorMessage);

// Функция для добавления файла в коллекцию анализируемых файлов
void addInputFileToModule(ProgramUnit* module, char* fileName, AstNode* ast);

// Функция для освобождения памяти, выделенной под модуль
void freeProgramUnit(ProgramUnit* programUnit);

extern InputFile* InputFiles;