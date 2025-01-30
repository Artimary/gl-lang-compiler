#pragma once
#include "model.h"

typedef struct CallEdge {
    char* caller;
    char* callee;
    struct CallEdge* next;
} CallEdge;

typedef struct CallGraph {
    char** functions;
    int funcCount;
    struct CallEdge* edges;
} CallGraph;

void initCallGraph(CallGraph* graph);
void addFunctionToGraph(CallGraph* graph, const char* funcName);
void addCallEdge(CallGraph* graph, const char* caller, const char* callee);
void drawCallGraph(CallGraph* graph, const char* filename);
void freeCallGraph(CallGraph* graph);