#include <stdio.h>
#include <stdlib.h>

#include "treeStructure.h"

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

int main(int argc, char* argv[]) {
	treeStruct* treeAST = treeGeneration("C:/SPO_labs/WinV/example.txt");
	//printf(psr->adaptor->makeDot(psr->adaptor, tree)->chars);
	//printf(treeAST->root);
	//if (treeAST != NULL) {
	//	freeEr();
	//}
    printTree(treeAST->tree, 0);
    FILE* dotFile = fopen("C:/SPO_labs/WinV/treeAST.dot", "w");
    //if (dotFile != NULL) {
        fprintf(dotFile, "digraph ParseTree {\n");
        fprintf(dotFile, "  node [shape=box];\n");
        int nodeCounter = 0;
        writeTreeAsDot(treeAST->tree, dotFile, &nodeCounter);
        fprintf(dotFile, "}\n");
        fclose(dotFile);
//        printf("Dot file generated: %s\n", argv[2]);
   // }
	//fclose(dotPath);
    if (treeAST->erCount != 0) {
        for (int i = 0; i < treeAST->erCount; i++) {
            fprintf(stderr, "ERROR: %s %s in input file %s\n", treeAST->errors[i]->message, treeAST->errors[i]->typeERR, treeAST->errors[i]->source);
        }
    }
    else printf("No errors found!");
	freeErrors(treeAST->errors, treeAST->erCount);
	//printf("Bu ispugalsa??\n");
    freeAST(treeAST->tree);
    free(treeAST);
	return 0;
}