#pragma once
#include "antlr/labgrammLexer.h"
#include "antlr/labgrammParser.h"
#include <antlr3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AstNode {
    struct AstNode** children;
    char* nodeName;
    int childrenCount;
} AstNode;

typedef struct errorInfo {
    char* message;
    char* typeERR;
    char* source;
} errorInfo;

typedef struct treeStruct{
    AstNode* tree;
    errorInfo** errors;
    int erCount;
} treeStruct;

char* replace_char(char* str, char find, char replace);
void printTree(AstNode* tree, int depth);
void freeErrors(errorInfo** errors, int erCount);
void freeAST(AstNode* tree);
treeStruct* treeGeneration(char* inputPath);
