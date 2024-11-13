
#include "treeStructure.h"

static errorInfo** errors = NULL;
static int errorCount = 0;
pANTLR3_BASE_TREE theBaseTree;
pANTLR3_STRING ttext;
pANTLR3_COMMON_TOKEN theToken;

static void printTree(pANTLR3_BASE_TREE tree, int depth) {
    if (tree == NULL) {
        return;
    }
    char* currentNode = (char*)tree->toString(tree)->chars;

    if (strcmp(currentNode, "ArrayTokenSuffix") == 0) {
        depth++;
    }

    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    unsigned int childCount = tree->getChildCount(tree);

    printf("%s\n", (char*)tree->toString(tree)->chars);

    for (unsigned int i = 0; i < childCount; i++) {
        if (strcmp(currentNode, "ArrayTokenSuffix") == 0) {
            printTree((pANTLR3_BASE_TREE)tree->getChild(tree, i), depth + 1);
        }
        else {
            printTree((pANTLR3_BASE_TREE)tree->getChild(tree, i), depth + 1);
        }
    }
}

static void error_list(int* size, errorInfo* newValue) {
    errorInfo** err = (errorInfo**)realloc(errors, (*size + 1) * sizeof(errorInfo*));
    if (err != NULL) {
        errors = err;
    }
    errors[*size] = (errorInfo*)malloc(sizeof(errorInfo));
    errors[*size] = newValue;
    (*size)++;
};

static void myReportError(ANTLR3_BASE_RECOGNIZER* recognizer) {
    errorInfo* error = (errorInfo*)malloc(sizeof(errorInfo));
    //errorCount = recognizer->state->errorCount;
    if (error) {
        theToken = (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);
        ttext = theToken->toString(theToken);
        error->source = recognizer->state->exception->streamName->chars;
        error->message = recognizer->state->exception->message;
        error->typeERR = ttext->chars;
        error_list(&errorCount, error);
    }
};

static void freeErrors() {
    for (int i = 0; i < errorCount; i++) {
        free(errors[i]);
    }
    free(errors);
};

static void makeDot(treeStruct* root, FILE* outFile) {
    fprintf(outFile, root);
    fclose(outFile);
}

static treeStruct* treeGeneration(char *inputPath, char* outputPath) {
    plabgrammLexer lxr = NULL;
    treeStruct* treeAST = malloc(sizeof(treeStruct));

    FILE* inputFile = fopen(inputPath, "r");
    if (inputFile == NULL) {
        printf("ERROR: unable to open input file \n");
        fclose(inputFile);
        lxr->free(lxr);
        return treeAST;
    }

    pANTLR3_INPUT_STREAM input = antlr3FileStreamNew((pANTLR3_UINT8)inputPath, ANTLR3_ENC_8BIT);
    if (input == NULL) {
        printf("ERROR: Could not open input stream.\n");
        fclose(inputFile);
        return treeAST;
    }

    lxr = labgrammLexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tokenStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    if (tokenStream == NULL) {
        printf("ERROR: unable to create token stream. \n");
        fclose(inputFile);
        tokenStream->free(tokenStream);
        lxr->free(lxr);
        input->close(input);
        return treeAST;
    }

    plabgrammParser psr = labgrammParserNew(tokenStream);
    if (psr == NULL) {
        printf("ERROR: unable to generate parser. \n");
        fclose(inputFile);
        psr->free(psr);
        tokenStream->free(tokenStream);
        lxr->free(lxr);
        input->close(input);
        return treeAST;
    }

    psr->pParser->rec->reportError = myReportError; //Ошибки при парсинге
    lxr->pLexer->rec->reportError = myReportError; //Ошибки лексики
    
    labgrammParser_source_return langAST = psr->source(psr);
    pANTLR3_BASE_TREE tree = langAST.tree;
    printTree(tree, 0);

    treeAST->root = psr->adaptor->makeDot(psr->adaptor, tree)->chars;
    FILE* dotPath = fopen(outputPath, "w");
    makeDot(treeAST->root, dotPath);

    
    treeAST->er = errors;
    treeAST->erCount = errorCount;
    
 
    if (treeAST->erCount != 0) {
        for (int i = 0; i < treeAST->erCount; i++) {
            fprintf(stderr, "ERROR: %s %s in input file %s\n", treeAST->er[i]->message, treeAST->er[i]->typeERR, treeAST->er[i]->source);
        }
    }
    else printf("No errors found!");

    psr->free(psr);
    tokenStream->free(tokenStream);
    lxr->free(lxr);
    input->close(input);
    fclose(inputFile);
    return treeAST;
};
