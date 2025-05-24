#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "treeStructure.h"
#include "cfg.h"
#include "model.h"
#include "codegen.h"

int main(int argc, char* argv[]) {
    // Генерация AST из файла
    treeStruct* treeAST = treeGeneration("C:/SPO_labs/WinV/example.txt");
    if (!treeAST) {
        printf("Ошибка: не удалось сгенерировать AST\n");
        return 1;
    }
    printTree(treeAST->tree, 0);

    // Создание DOT-файла для AST
    FILE* dotFile = fopen("C:/SPO_labs/WinV/treeAST.dot", "w");
    if (!dotFile) {
        printf("Ошибка: не удалось открыть файл treeAST.dot\n");
        freeAST(treeAST->tree);
        free(treeAST);
        return 1;
    }
    fprintf(dotFile, "digraph ParseTree {\n");
    fprintf(dotFile, "  node [shape=box];\n");
    int nodeCounter = 0;
    writeTreeAsDot(treeAST->tree, dotFile, &nodeCounter);
    fprintf(dotFile, "}\n");
    fclose(dotFile);

    // Создание и заполнение ProgramUnit
    ProgramUnit* programUnit = createModule("example.txt");
    if (!programUnit) {
        printf("Ошибка: не удалось создать ProgramUnit\n");
        freeAST(treeAST->tree);
        free(treeAST);
        return 1;
    }
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

    // Генерация образа программы с помощью codegen
    ProgramImage* programImage = generateProgramImage(programUnit);
    if (!programImage) {
        printf("Ошибка: не удалось сгенерировать ProgramImage\n");
    }
    else {
        // Вывод сгенерированного кода для отладки
        for (int i = 0; i < programImage->func_count; i++) {
            FunctionCode* fc = &programImage->functions[i];
            printf("Function: %s\n", fc->name);
            for (int j = 0; j < fc->instr_count; j++) {
                Instruction* instr = &fc->instructions[j];
                printf("  0x%lx: %s", instr->address, instr->mnemonic);
                if (instr->operands[0]) printf(" %s", instr->operands[0]);
                if (instr->operands[1]) printf(", %s", instr->operands[1]);
                if (instr->operands[2]) printf(", %s", instr->operands[2]);
                printf("\n");
            }
            printf("\n");
        }

        // Освобождение памяти для ProgramImage
        freeProgramImage(programImage);
    }

    // Отрисовка графа вызовов
    drawCallGraph(&programUnit->callGraph, "call_graph.dot");

    // Очистка памяти
    freeAST(treeAST->tree);
    free(treeAST);
    freeProgramUnit(programUnit);

    return 0;
}