#pragma once
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
    ProgramUnit* module = calloc(1, sizeof(ProgramUnit));
    if (!module) return NULL;

    module->sourceFileName = strdup(sourceFileName);
    initCallGraph(&module->callGraph);

    module->funcs = NULL;
    module->funcsCount = 0;
    module->inputFiles = NULL;
    module->inputFilesCount = 0;
    module->errors.error_messages = NULL;
    module->errors.errors_count = 0;

    return module;
}

void processFuncDef(AstNode* funcDefNode, ProgramUnit* programUnit) {
    if (strcmp(funcDefNode->nodeName, "FuncDef") != 0) return;

    AstNode* funcSignature = NULL;
    AstNode* body = NULL;

    for (int i = 0; i < funcDefNode->childrenCount; i++) {
        AstNode* child = funcDefNode->children[i];
        if (strcmp(child->nodeName, "FuncSignature") == 0) {
            funcSignature = child;
        }
        else if (strcmp(child->nodeName, "Body") == 0) {
            body = child;
        }
    }

    if (!funcSignature || !body) {
        fprintf(stderr, "ERROR: Invalid FuncDef structure.\n");
        return;
    }

    char* funcName = NULL;
    for (int i = 0; i < funcSignature->childrenCount; i++) {
        AstNode* child = funcSignature->children[i];
        if (strcmp(child->nodeName, "Identifier") == 0 && child->childrenCount > 0) {
            funcName = strdup(child->children[0]->nodeName);
            break;
        }
    }

    if (!funcName) {
        fprintf(stderr, "ERROR: Failed to extract function name.\n");
        return;
    }

    CfgInstance* cfg = prepareControlFlowGraph(programUnit, body);
    if (!cfg) {
        return;
    }

    CfgContext ctx = {
        .cfg = createCfgInstance(),
        .programUnit = programUnit,
        .currentFunction = funcName,
        .currentNode = NULL,
        .loopAfterStack = NULL,
        .loopAfterStackSize = 0
    };

    addFunctionToGraph(&programUnit->callGraph, funcName);

    collectCfg(body, &ctx);
    

    FuncDefN funcDef;
    funcDef.name = funcName;
    funcDef.cfg = ctx.cfg;
    funcDef.signature = NULL;

    addFunctionToModule(programUnit, &funcDef);

    drawCFG(ctx.cfg, funcName);
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
    if (!module || !fileName || !ast) return;

    InputFile* newFiles = realloc(module->inputFiles,
        (module->inputFilesCount + 1) * sizeof(InputFile));
    if (!newFiles) {
        fprintf(stderr, "ERROR: Memory allocation failed for input files\n");
        return;
    }

    module->inputFiles = newFiles;
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
    if (!programUnit) return;

    for (int i = 0; i < programUnit->funcsCount; i++) {
        if (programUnit->funcs[i].name) {
            free(programUnit->funcs[i].name);
        }
        if (programUnit->funcs[i].cfg) {
            freeCfg(programUnit->funcs[i].cfg);
        }
    }
    free(programUnit->funcs);
    freeCallGraph(&programUnit->callGraph);

    for (int i = 0; i < programUnit->errors.errors_count; i++) {
        free(programUnit->errors.error_messages[i]);
    }
    free(programUnit->errors.error_messages);

    free(programUnit->sourceFileName);
    free(programUnit);
}