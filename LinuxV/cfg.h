#pragma once
#include "treeStructure.h"
#include "OpTree.h"
#include "model.h"

typedef struct ProgramUnit ProgramUnit;

typedef struct CfgNode CfgNode;
typedef struct CfgInstance CfgInstance;
typedef struct CfgContext CfgContext;

struct CfgNode {
    int id;
    char* nodeName;
    OpNode* optree;
    CfgNode* nextNode;
    CfgNode* condNode;
};

struct CfgInstance {
    CfgNode** nodes;
    int count;
    int capacity;
};

struct CfgContext {
    CfgInstance* cfg;
    ProgramUnit* programUnit;
    char* currentFunction;
    CfgNode** loopAfterStack;
    int loopAfterStackSize;
    CfgNode** loopCurrentStack;
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

    