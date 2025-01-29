// main.c
#include <stdio.h>
#include <stdlib.h>
#include "treeStructure.h"
#include "cfg.h"
#include "model.h"

void writeTreeAsDot(AstNode* node, FILE* file, int* nodeCounter) {
    if (node == NULL) {
        return;
    }

    if (strchr(node->nodeName, '"') != 0) {
        replace_char(node->nodeName, '"', ' ');
    }
    int currentNodeId = (*nodeCounter)++;
    fprintf(file, "  node%d [label=\"%s\"];\n", currentNodeId, node->nodeName);

    for (int i = 0; i < node->childrenCount; i++) {
        int childNodeId = *nodeCounter;
        writeTreeAsDot(node->children[i], file, nodeCounter);
        fprintf(file, "  node%d -> node%d;\n", currentNodeId, childNodeId);
    }
}

ProgramUnit* createModule(char* sourceFileName) {
    ProgramUnit* module = (ProgramUnit*)malloc(sizeof(ProgramUnit));
    if (module == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for ProgramUnit.\n");
        return NULL;
    }

    module->funcs = NULL;
    module->funcsCount = 0;
    module->inputFiles = NULL;
    module->inputFilesCount = 0;
    module->sourceFileName = strdup(sourceFileName);  // Копируем имя файла
    if (module->sourceFileName == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for sourceFileName.\n");
        free(module);
        return NULL;
    }

    module->errors.errors_count = 0;
    module->errors.error_messages = NULL;

    return module;
}
void addFunctionToModule(ProgramUnit* module, FuncDefN* func) {
    if (module == NULL || func == NULL) {
        return;
    }

    module->funcs = (FuncDefN*)realloc(module->funcs, (module->funcsCount + 1) * sizeof(FuncDefN));
    if (module->funcs == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for functions.\n");
        return;
    }

    module->funcs[module->funcsCount] = *func;
    module->funcsCount++;
}

void addInputFileToModule(ProgramUnit* module, char* fileName, AstNode* ast) {
    if (module == NULL || fileName == NULL || ast == NULL) {
        return;
    }

    module->inputFiles = (InputFile*)realloc(module->inputFiles, (module->inputFilesCount + 1) * sizeof(InputFile));
    if (module->inputFiles == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for input files.\n");
        return;
    }

    module->inputFiles[module->inputFilesCount].fileName = strdup(fileName);
    module->inputFiles[module->inputFilesCount].ast = ast;
    module->inputFilesCount++;
}

void addErrorToModule(ProgramUnit* module, char* errorMessage) {
    if (module == NULL || errorMessage == NULL) {
        return;
    }

    module->errors.error_messages = (char**)realloc(module->errors.error_messages, (module->errors.errors_count + 1) * sizeof(char*));
    if (module->errors.error_messages == NULL) {
        fprintf(stderr, "ERROR: Unable to allocate memory for error messages.\n");
        return;
    }

    module->errors.error_messages[module->errors.errors_count] = strdup(errorMessage);
    module->errors.errors_count++;
}

void freeProgramUnit(ProgramUnit* programUnit) {
    if (programUnit == NULL) {
        return;
    }

    for (int i = 0; i < programUnit->funcsCount; i++) {
        free(programUnit->funcs[i].name);
        free(programUnit->funcs[i].signature);
        // Освобождение памяти для других полей FuncDefN, если необходимо
    }
    free(programUnit->funcs);

    for (int i = 0; i < programUnit->inputFilesCount; i++) {
        free(programUnit->inputFiles[i].fileName);
        // Освобождение памяти для AST, если необходимо
    }
    free(programUnit->inputFiles);

    for (int i = 0; i < programUnit->errors.errors_count; i++) {
        free(programUnit->errors.error_messages[i]);
    }
    free(programUnit->errors.error_messages);

    free(programUnit->sourceFileName);  // Освобождаем память для sourceFileName
    free(programUnit);
}

int main(int argc, char* argv[]) {
    treeStruct* treeAST = treeGeneration("C:/SPO_labs/WinV/example.txt");
    printTree(treeAST->tree, 0);
    FILE* dotFile = fopen("C:/SPO_labs/WinV/treeAST.dot", "w");
    fprintf(dotFile, "digraph ParseTree {\n");
    fprintf(dotFile, "  node [shape=box];\n");
    int nodeCounter = 0;
    writeTreeAsDot(treeAST->tree, dotFile, &nodeCounter);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    ProgramUnit* programUnit = createModule("C:/SPO_labs/WinV/example.txt");

    // Добавляем файл в коллекцию
    addInputFileToModule(programUnit, "C:/SPO_labs/WinV/example.txt", treeAST->tree);

    CfgInstance* cfg = prepareControlFlowGraph(programUnit, treeAST->tree);
    drawCFG(cfg, "main");

    freeCfg(cfg);

    freeErrors(treeAST->errors, treeAST->erCount);
    freeAST(treeAST->tree);
    free(treeAST);

    freeProgramUnit(programUnit);
    return 0;
}