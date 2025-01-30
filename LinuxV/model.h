#pragma once
#include "cfg.h"
#include "callGraph.h" 

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
    struct CfgInstance* cfg;
};

struct ProgramUnit {
    FuncDefN* funcs;
    int funcsCount;
    struct CallGraph* callGraph;
    InputFile* inputFiles;
    int inputFilesCount;
    char* sourceFileName;
    struct {
        int errors_count;
        char** error_messages;
    } errors;
};

struct InputFile {
    char* fileName;
    AstNode* ast;
};

void writeTreeAsDot(AstNode* node, FILE* file, int* nodeCounter);
ProgramUnit* createModule(char* sourceFileName);
void processFuncDef(AstNode* funcDefNode, ProgramUnit* programUnit);
void addFunctionToModule(ProgramUnit* module, FuncDefN* func);
void addInputFileToModule(ProgramUnit* module, char* fileName, AstNode* ast);
void addErrorToModule(ProgramUnit* module, char* errorMessage);
void freeProgramUnit(ProgramUnit* programUnit);

extern InputFile* InputFiles;