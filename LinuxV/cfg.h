#pragma once
#include "treeStructure.h"
#include "OpTree.h"

typedef struct CfgNode CfgNode;
typedef struct CfgInstance CfgInstance;
typedef struct CfgContext CfgContext;


struct CfgNode {
    int id;
    char* nodeName;
    OpNode* optree;
    CfgNode* nextNode; // Следующий узел
    CfgNode* condNode; // Узел условия (для if, while, for)
    CfgInstance* nodes;
};

struct CfgInstance {
    CfgNode** nodes;       // Массив узлов CFG
    int count;             // Текущее количество узлов
    int capacity;          // Вместимость массива узлов
};

struct CfgContext {
    CfgInstance* cfg;            // Текущий экземпляр CFG
    CfgNode** loopAfterStack;    // Стек узлов для выхода из циклов (после тела)
    int loopAfterStackSize;
    CfgNode** loopCurrentStack;  // Стек текущих циклов
    int loopCurrentStackSize;
    CfgNode* currentNode;
};

// Функции для работы с CFG
CfgNode* collectCfg(AstNode* node, CfgContext* ctx);
CfgNode* createCfgNode(CfgContext* ctx, const char* nodeName);
CfgInstance* prepareControlFlowGraph(struct ProgramUnit* model, AstNode* stat);
CfgInstance* buildCfgForFunction(AstNode* functionNode);
CfgInstance* createCfgInstance();

// Функции для обработки узлов AST
CfgNode* handleIfStatement(AstNode* node, CfgContext* ctx);
CfgNode* handleWhileStatement(AstNode* node, CfgContext* ctx);
CfgNode* handleReturnStatement(AstNode* node, CfgContext* ctx);
CfgNode* handleFunctionCall(AstNode* node, CfgContext* ctx);
CfgNode* handleExpression(AstNode* node, CfgContext* ctx);
CfgNode* handleBreakStatement(AstNode* node, CfgContext* ctx);

void writeCfgToDot(CfgInstance* cfg, const char* filename);
void freeCfg(CfgInstance* cfg);

