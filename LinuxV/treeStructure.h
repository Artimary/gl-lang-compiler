#pragma once
#include "antlr/labgrammLexer.h"
#include "antlr/labgrammParser.h"
#include <antlr3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct errorInfo {
    char* message;
    char* typeERR;
    char* source;
} errorInfo;

typedef struct treeStruct{
    char* root;
    struct errorInfo** er;
    int erCount;
} treeStruct;

