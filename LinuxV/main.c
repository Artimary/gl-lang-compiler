#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "treeStructure.h"
#include "cfg.h"
#include "model.h"

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

    ProgramUnit* programUnit = createModule("example.txt");
    addInputFileToModule(programUnit, "example.txt", treeAST->tree);

    // Инициализация графа вызовов
    initCallGraph(&programUnit->callGraph);

    // Обработка всех функций в AST
    AstNode* sourceNode = treeAST->tree;
    for (int i = 0; i < sourceNode->childrenCount; i++) {
        AstNode* child = sourceNode->children[i];
        if (strcmp(child->nodeName, "FuncDef") == 0) {
            processFuncDef(child, programUnit);
        }
    }

    drawCallGraph(&programUnit->callGraph, "call_graph.dot");

    freeAST(treeAST->tree);
    free(treeAST);
    freeProgramUnit(programUnit);
    return 0;
}