#include "callgraph.h"
#include <stdlib.h>
#include <string.h>

void initCallGraph(CallGraph* graph) {
    graph->functions = NULL;
    graph->funcCount = 0;
    graph->edges = NULL;
}

void addFunctionToGraph(CallGraph* graph, const char* funcName) {
    for (int i = 0; i < graph->funcCount; i++) {
        if (strcmp(graph->functions[i], funcName) == 0) return;
    }

    graph->functions = realloc(graph->functions, (graph->funcCount + 1) * sizeof(char*));
    graph->functions[graph->funcCount] = strdup(funcName);
    graph->funcCount++;
}

void addCallEdge(CallGraph* graph, const char* caller, const char* callee) {
    struct CallEdge* current = graph->edges;
    while (current != NULL) {
        if (strcmp(current->caller, caller) == 0 && strcmp(current->callee, callee) == 0) {
            return;
        }
        current = current->next;
    }

    struct CallEdge* newEdge = malloc(sizeof(struct CallEdge));
    newEdge->caller = strdup(caller);
    newEdge->callee = strdup(callee);
    newEdge->next = graph->edges;
    graph->edges = newEdge;
}

void drawCallGraph(CallGraph* graph, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) return;

    fprintf(fp, "digraph CallGraph {\n");
    fprintf(fp, "  node [shape=circle, style=filled, fillcolor=lightblue];\n");

    for (int i = 0; i < graph->funcCount; i++) {
        fprintf(fp, "  \"%s\";\n", graph->functions[i]);
    }

    struct CallEdge* edge = graph->edges;
    while (edge) {
        fprintf(fp, "  \"%s\" -> \"%s\";\n", edge->caller, edge->callee);
        edge = edge->next;
    }

    fprintf(fp, "}\n");
    fclose(fp);
}

void freeCallGraph(CallGraph* graph) {
    for (int i = 0; i < graph->funcCount; i++) {
        free(graph->functions[i]);
    }
    free(graph->functions);

    struct CallEdge* edge = graph->edges;
    while (edge) {
        struct CallEdge* next = edge->next;
        free(edge->caller);
        free(edge->callee);
        free(edge);
        edge = next;
    }
}