#pragma once
#include "treeStructure.h"


errorInfo** errors = NULL;
int errorCount = 0;
pANTLR3_BASE_TREE theBaseTree;
pANTLR3_STRING ttext;
pANTLR3_COMMON_TOKEN theToken;

AstNode* tmp;