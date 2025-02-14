#include "cfg.h"
#include "treeStructure.h"
#include "model.h"
#include "callGraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CfgInstance* createCfgInstance() {
    CfgInstance* cfg = (CfgInstance*)malloc(sizeof(CfgInstance));
    cfg->nodes = NULL;
    cfg->count = 0;
    cfg->capacity = 0;
    return cfg;
}

CfgNode* createCfgNode(CfgContext* ctx, const char* nodeName) {
    CfgNode* node = (CfgNode*)malloc(sizeof(CfgNode));
    node->id = ctx->cfg->count;
    node->nodeName = strdup(nodeName);
    node->nextNode = NULL;
    node->condNode = NULL;
    node->optree = NULL;

    if (ctx->cfg->count >= ctx->cfg->capacity) {
        int newCapacity = ctx->cfg->capacity == 0 ? 4 : ctx->cfg->capacity * 2;
        ctx->cfg->nodes = (CfgNode**)realloc(ctx->cfg->nodes, newCapacity * sizeof(CfgNode*));
        ctx->cfg->capacity = newCapacity;
    }
    ctx->cfg->nodes[ctx->cfg->count++] = node;
    return node;
}

CfgNode* handleIfStatement(AstNode* node, CfgContext* ctx) {
    CfgNode* ifNode = createCfgNode(ctx, "If");
    CfgNode* thenNode = createCfgNode(ctx, "Then");
    CfgNode* elseNode = node->childrenCount > 2 ? createCfgNode(ctx, "Else") : NULL;
    CfgNode* mergeNode = createCfgNode(ctx, "Merge");

    AstNode* condExpr = node->children[0];
    ifNode->optree = buildOpTree(condExpr);

    ifNode->condNode = thenNode;
    ifNode->nextNode = elseNode ? elseNode : mergeNode;

    CfgNode* prevCurrent = ctx->currentNode;
    ctx->currentNode = thenNode;
    collectCfg(node->children[1], ctx);
    CfgNode* thenLast = ctx->currentNode;

    CfgNode* elseLast = NULL;
    if (elseNode) {
        ctx->currentNode = elseNode;
        collectCfg(node->children[2], ctx);
        elseLast = ctx->currentNode;
    }

    ctx->currentNode = prevCurrent;

    if (thenLast) thenLast->nextNode = mergeNode;
    if (elseLast) elseLast->nextNode = mergeNode;

    if (ctx->currentNode) {
        ctx->currentNode->nextNode = ifNode;
    }
    ctx->currentNode = mergeNode;

    return ifNode;
}

CfgNode* handleBreakStatement(AstNode* node, CfgContext* ctx) {
    if (ctx->loopAfterStackSize == 0) return NULL;

    CfgNode* breakNode = createCfgNode(ctx, "Break");
    CfgNode* loopEnd = ctx->loopAfterStack[ctx->loopAfterStackSize - 1];

    if (ctx->currentNode) {
        ctx->currentNode->nextNode = breakNode;
    }
    breakNode->nextNode = loopEnd;
    ctx->currentNode = NULL;

    return breakNode;
}

CfgNode* handleFunctionCall(AstNode* node, CfgContext* ctx) {
    CfgNode* callNode = createCfgNode(ctx, "Call");

    AstNode* funcNameNode = node->children[0];
    char* callee = strdup(funcNameNode->children[0]->nodeName);

    if (ctx->currentFunction && ctx->programUnit) {
        addFunctionToGraph(&ctx->programUnit->callGraph, ctx->currentFunction);
        addFunctionToGraph(&ctx->programUnit->callGraph, callee);

        addCallEdge(&ctx->programUnit->callGraph, ctx->currentFunction, callee);
    }
    callNode->optree = buildOpTree(node);
    if (ctx->currentNode) {
        ctx->currentNode->nextNode = callNode;
    }
    ctx->currentNode = callNode;

    free(callee);
    return callNode;
}

CfgNode* handleWhileStatement(AstNode* node, CfgContext* ctx) {
    CfgNode* loopEntry = createCfgNode(ctx, "LoopEntry");
    CfgNode* loopCond = createCfgNode(ctx, "LoopCond");
    CfgNode* loopBody = createCfgNode(ctx, "LoopBody");
    CfgNode* loopEnd = createCfgNode(ctx, "LoopEnd");

    AstNode* condExpr = node->children[0];
    loopCond->optree = buildOpTree(condExpr);

    if (ctx->currentNode) ctx->currentNode->nextNode = loopEntry;
    loopEntry->nextNode = loopCond;
    loopCond->condNode = loopBody;
    loopCond->nextNode = loopEnd;

    ctx->loopAfterStack = realloc(ctx->loopAfterStack, sizeof(CfgNode*) * (ctx->loopAfterStackSize + 1));
    ctx->loopAfterStack[ctx->loopAfterStackSize++] = loopEnd;

    CfgNode* prevCurrent = ctx->currentNode;
    ctx->currentNode = loopBody;
    collectCfg(node->children[1], ctx);

    if (ctx->currentNode && ctx->currentNode->nextNode == NULL) {
        ctx->currentNode->nextNode = loopCond;
    }

    ctx->currentNode = loopEnd;
    ctx->loopAfterStackSize--;

    return loopEntry;
}

CfgNode* handleRepeatStatement(AstNode* node, CfgContext* ctx) {
    CfgNode* loopEntry = createCfgNode(ctx, "RepeatEntry");
    CfgNode* loopBody = createCfgNode(ctx, "LoopBody");
    CfgNode* loopCond = createCfgNode(ctx, "LoopCond");
    CfgNode* loopEnd = createCfgNode(ctx, "LoopEnd");

    if (ctx->currentNode) {
        ctx->currentNode->nextNode = loopEntry;
    }
    loopEntry->nextNode = loopBody;

    ctx->loopAfterStack = realloc(ctx->loopAfterStack, sizeof(CfgNode*) * (ctx->loopAfterStackSize + 1));
    ctx->loopAfterStack[ctx->loopAfterStackSize++] = loopEnd;

    CfgNode* prevCurrent = ctx->currentNode;
    ctx->currentNode = loopBody;
    collectCfg(node->children[0], ctx);

    if (ctx->currentNode && ctx->currentNode->nextNode == NULL) {
        ctx->currentNode->nextNode = loopCond;
    }

    loopCond->condNode = loopEntry;
    loopCond->nextNode = loopEnd;

    ctx->currentNode = loopEnd;
    ctx->loopAfterStackSize--;

    return loopEntry;
}

CfgNode* handleReturnStatement(AstNode* node, CfgContext* ctx) {
    CfgNode* retNode = createCfgNode(ctx, "Return");
    if (ctx->currentNode) {
        ctx->currentNode->nextNode = retNode;
    }
    retNode->nextNode = NULL;
    ctx->currentNode = NULL;
    return retNode;
}

CfgNode* collectCfg(AstNode* node, CfgContext* ctx) {
    if (!node || !node->nodeName) return NULL;

    printf("Processing node: %s\n", node->nodeName);

    if (strcmp(node->nodeName, "ConditionStatement") == 0) {
        CfgNode* ifNode = handleIfStatement(node, ctx);
        return ifNode;
    }
    else if (strcmp(node->nodeName, "LoopStatement") == 0) {
        CfgNode* loopNode = handleWhileStatement(node, ctx);
        return loopNode;
    }
    else if (strcmp(node->nodeName, "ReturnStatement") == 0) {
        return handleReturnStatement(node, ctx);
    }
    else if (strcmp(node->nodeName, "BreakStatement") == 0) {
        return handleBreakStatement(node, ctx);
    }
    else if (strcmp(node->nodeName, "Call") == 0) {
        return handleFunctionCall(node, ctx);
    }
    else if (strcmp(node->nodeName, "RepeatStatement") == 0) {
        CfgNode* repeatNode = handleRepeatStatement(node, ctx);
        return repeatNode;
    }
    else if (strcmp(node->nodeName, "VarDeclaration") == 0) {
        CfgNode* cfgNode = createCfgNode(ctx, node->nodeName);
        if (ctx->currentNode) {
            ctx->currentNode->nextNode = cfgNode;
        }
        ctx->currentNode = cfgNode;
        return cfgNode;
    }
    else if (strcmp(node->nodeName, "AssignmentOP") == 0) {
        CfgNode* cfgNode = createCfgNode(ctx, "AssignmentOP");

        if (node->childrenCount > 0) {
            AstNode* assignmentExpr = node->children[0];
            cfgNode->optree = buildOpTree(assignmentExpr);
        }

        if (ctx->currentNode) ctx->currentNode->nextNode = cfgNode;
        ctx->currentNode = cfgNode;
        return cfgNode;
    }

    for (int i = 0; i < node->childrenCount; i++) {
        collectCfg(node->children[i], ctx);
    }

    return NULL;
}

CfgInstance* prepareControlFlowGraph(ProgramUnit* model, AstNode* stat) {
    if (model == NULL || stat == NULL) {
        return NULL;
    }

    printf("Preparing CFG for file: %s\n", model->sourceFileName);

    CfgInstance* cfg = createCfgInstance();
    CfgContext ctx = {
        .cfg = cfg,
        .loopCurrentStack = NULL,
        .loopAfterStack = NULL,
        .currentNode = NULL
    };
    collectCfg(stat, &ctx);
    return cfg;
}

void drawCFG(CfgInstance* cfg, const char* functionName) {
    printf("[DEBUG] Drawing CFG for %s\n", functionName);
    if (!cfg || !functionName) return;

    char fname[256];
    snprintf(fname, sizeof(fname), "%s.dot", functionName);

    FILE* fp = fopen(fname, "w");
    if (!fp) return;

    fprintf(fp, "digraph CFG {\n");
    fprintf(fp, "    node [shape=box];\n");
    fprintf(fp, "    label=\"%s\";\n", functionName);
    fprintf(fp, "    labelloc=t;\n");

    fprintf(fp, "    start [shape=Mdiamond];\n");
    fprintf(fp, "    end [shape=Msquare];\n");

    for (int i = 0; i < cfg->count; i++) {
        CfgNode* node = cfg->nodes[i];
        fprintf(fp, "    node%d [label=\"%s", node->id, node->nodeName);

        if (node->optree) {
            char* expr = opTreeToString(node->optree);
            if (expr != NULL && expr[0] != '\0') {
                if (strchr(expr, '"') != 0) {
                    replace_char(expr, '"', '\'');
                }
                fprintf(fp, "\\n%s", expr);
                free(expr);
            }
            else {
                fprintf(fp, "\\n[empty]");
                if (expr) free(expr);
            }
        }

        fprintf(fp, "\"];\n");
    }

    for (int i = 0; i < cfg->count; i++) {
        CfgNode* node = cfg->nodes[i];

        if (strcmp(node->nodeName, "Break") == 0 && node->nextNode) {
            fprintf(fp, "    node%d -> node%d [label=\"break\", style=dashed];\n", node->id, node->nextNode->id);
        }
        else if (node->nextNode) {
            fprintf(fp, "    node%d -> node%d \n", node->id, node->nextNode->id);
        }
        if (node->condNode) {
            fprintf(fp, "    node%d -> node%d [label=\"true\"];\n", node->id, node->condNode->id);
        }
    }

    int* exitNodes = calloc(cfg->count, sizeof(int));
    for (int i = 0; i < cfg->count; i++) {
        CfgNode* node = cfg->nodes[i];
        if (!node->nextNode && !node->condNode) {
            exitNodes[i] = 1;
        }
    }

    for (int i = 0; i < cfg->count; i++) {
        if (exitNodes[i]) {
            fprintf(fp, "    node%d -> end;\n", cfg->nodes[i]->id);
        }
    }
    free(exitNodes);

    if (cfg->count > 0) {
        fprintf(fp, "    start -> node0;\n");
    }

    fprintf(fp, "}\n");
    fclose(fp);
}

void freeCfg(CfgInstance* cfg) {
    if (!cfg) return;
    for (int i = 0; i < cfg->count; i++) {
        free(cfg->nodes[i]->nodeName);
        freeOpTree(cfg->nodes[i]->optree);
        free(cfg->nodes[i]);
    }
    free(cfg->nodes);
    free(cfg);
}