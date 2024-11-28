
#include "define.h"

#include "assert.h"
#include <stdio.h>


char* replace_char(char* str, char find, char replace) {
    char* current_pos = strchr(str, find);
    do {
        *current_pos = replace;
        current_pos = strchr(current_pos, find);
    } while (current_pos);
    return str;
}

void printTree(AstNode* tree, int depth) {
    if (tree == NULL) {
        return;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    
    //if (tree->childrenCount < 0) tree->childrenCount = 0;

    printf("%s\n", tree->nodeName);

    if (strcmp(tree->nodeName, "Array") == 0) {
        for (int i = tree->childrenCount; i > 0; i--)
            printTree((AstNode*)tree->children[i - 1], depth + 1);
    }
    else{
    for (int i = 0; i < tree->childrenCount; i++)
        printTree((AstNode*)tree->children[i], depth + 1);
    }
}

AstNode* convertToMyTree(pANTLR3_BASE_TREE antlrTree) {
    if (antlrTree == NULL) {
        return NULL;
    }

    char* currentNode = (char*)antlrTree->toString(antlrTree)->chars;
    int count = antlrTree->getChildCount(antlrTree);
    AstNode* result = malloc(sizeof(AstNode));
    assert(result);
    
    //Сортировка под TypeRef
    if (strcmp(currentNode, "TypeRef") == 0) {
        pANTLR3_BASE_TREE basic_antlr_type = (pANTLR3_BASE_TREE)antlrTree->getChild(antlrTree, 0);
        pANTLR3_BASE_TREE basic_typeref_value = (pANTLR3_BASE_TREE)basic_antlr_type->getChild(basic_antlr_type, 0);
        
        
        AstNode* tpref = malloc(sizeof(AstNode));
        assert(tpref);
        result->nodeName = strdup(basic_typeref_value->toString(basic_typeref_value)->chars);
        result->childrenCount = basic_typeref_value->getChildCount(basic_typeref_value);

        tmp = malloc(sizeof(AstNode));
        assert(tmp);
        tmp->children = malloc(sizeof(AstNode*));
        tmp->nodeName = strdup(basic_antlr_type->toString(basic_antlr_type)->chars);
        assert(tmp->children);
        tmp->children[0] = result;
        tmp->childrenCount = 1;

        tpref->children = malloc(sizeof(AstNode*));
        assert(tpref->children);

        tpref->nodeName = strdup("TypeRef");
        tpref->childrenCount = 1;
        tpref->children[0] = tmp;

        result = tpref;
        
        result->childrenCount = 1;

        //Случай Array
        if (count > 1) {
            for (int i = 1; i < count; ++i) {
                pANTLR3_BASE_TREE arr = (pANTLR3_BASE_TREE)antlrTree->getChild(antlrTree, i);
                int dim = arr->getChildCount(arr) + 1;

                tmp = malloc(sizeof(AstNode));
                tpref = malloc(sizeof(AstNode));
                assert(tmp);
                assert(tpref);
                tmp->children = malloc(sizeof(AstNode*) * 2);
                tpref->children = malloc(sizeof(AstNode*) * 2);
                assert(tmp->children);
                assert(tpref->children);
                tmp->childrenCount = 2;
                tmp->nodeName = strdup(arr->toString(arr)->chars);
                tmp->children[0] = result;

                tmp->children[1] = malloc(sizeof(AstNode));
                assert(tmp->children[1]);

                char* buf = malloc(21);
                assert(buf);
                buf[20] = '\0';
               
                sprintf(buf, "%d", dim);

                tmp->children[1]->nodeName = buf;
                tmp->children[1]->childrenCount = 0;

                tpref->nodeName = strdup("TypeRef");
                tpref->childrenCount = 1;
                tpref->children[0] = tmp;


                result = tpref;
            }
        }

        return result;
    }
    
    //AstNode* node = malloc(sizeof(AstNode));
    //assert(node);
    result->childrenCount = antlrTree->getChildCount(antlrTree);;
    result->children = malloc(sizeof(AstNode*) * result->childrenCount);
    assert(result->children);
    if (result->childrenCount == 0) result->childrenCount = 0;

    pANTLR3_STRING nodeText = antlrTree->toString(antlrTree);
    result->nodeName = strdup((char*)nodeText->chars);

    if (result->childrenCount > 0) {
        for (int i = 0; i < result->childrenCount; i++) {
            pANTLR3_BASE_TREE childTree = (pANTLR3_BASE_TREE)antlrTree->getChild(antlrTree, i);
            result->children[i] = convertToMyTree(childTree);
        }
    }
    else {
        result->children = NULL;
    }

    return result;
    
};

void error_list(int* size, errorInfo* newValue) {
    errorInfo** err = (errorInfo**)realloc(errors, (*size + 1) * sizeof(errorInfo*));
    if (err != NULL) {
        errors = err;
    }
    errors[*size] = (errorInfo*)malloc(sizeof(errorInfo));
    errors[*size] = newValue;
    (*size)++;
};

void myReportError(ANTLR3_BASE_RECOGNIZER* recognizer) {
    errorInfo* error = (errorInfo*)malloc(sizeof(errorInfo));
    if (error) {
        theToken = (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);
        ttext = theToken->toString(theToken);
        error->source = recognizer->state->exception->streamName->chars;
        error->message = recognizer->state->exception->message;
        error->typeERR = ttext->chars;
        error_list(&errorCount, error);
    }
};

void freeErrors(errorInfo** errors, int erCount) {
    for (int i = 0; i < erCount; i++) {
        free(errors[i]);
    }
    free(errors);
};

void freeAST(AstNode* tree) {
    if (tree == NULL) {
        return;
    }
    if (tree->childrenCount > 0) {
        for (int i = 0; i < tree->childrenCount; ++i) {
            freeAST(tree->children[i]);
        }

        free(tree->children);
    }
    free(tree->nodeName);
    free(tree);
}

treeStruct* treeGeneration(char *inputPath) {
    plabgrammLexer lxr;
    treeStruct* treeAST = malloc(sizeof(treeStruct));
    assert(treeAST);
    printf("%s\n",inputPath);
    FILE* inputFile = fopen(inputPath, "r+");
    if (inputFile == NULL) {
        fprintf(stderr,"ERROR: unable to open input file \n");
        fclose(inputFile);
        return treeAST;
    }

    pANTLR3_INPUT_STREAM input = antlr3FileStreamNew((pANTLR3_UINT8)inputPath, ANTLR3_ENC_8BIT);
    if (input == NULL) {
        fprintf(stderr,"ERROR: Could not open input stream.\n");
        fclose(inputFile);
        return treeAST;
    }

    lxr = labgrammLexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    if (tokenStream == NULL) {
        fprintf(stderr,"ERROR: unable to create token stream. \n");
        fclose(inputFile);
        lxr->free(lxr);
        input->close(input);
        return treeAST;
    }

    plabgrammParser psr = labgrammParserNew(tokenStream);
    if (psr == NULL) {
        fprintf(stderr,"ERROR: unable to generate parser. \n");
        fclose(inputFile);
        tokenStream->free(tokenStream);
        lxr->free(lxr);
        input->close(input);
        return treeAST;
    }

    psr->pParser->rec->reportError = myReportError; //Ошибки при парсинге
    lxr->pLexer->rec->reportError = myReportError; //Ошибки лексики
    
    labgrammParser_source_return langAST = psr->source(psr);
    pANTLR3_BASE_TREE tree = langAST.tree;
    
    //Переопределяем дерево в своей структуре
    treeAST->tree = convertToMyTree(tree);
    
    treeAST->errors = errors;
    treeAST->erCount = errorCount;
    
    psr->free(psr);
    tokenStream->free(tokenStream);
    lxr->free(lxr);
    input->close(input);
    fclose(inputFile);
    
    return treeAST;
};
