#include "OpTree.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int is_integer(const char* str) {
    if (str == NULL || *str == '\0') return 0;

    size_t start = 0;

    if (str[0] == '-' || str[0] == '+') {
        start = 1;
        if (str[1] == '\0') return 0;
    }

    for (size_t i = start; i < strlen(str); i++) {
        if (!isdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

int is_float(const char* str) {
    if (str == NULL || *str == '\0') return 0;

    int has_dot = 0;
    int has_exp = 0;
    int has_digit = 0;
    size_t start = 0;

    if (str[0] == '-' || str[0] == '+') {
        start = 1;
    }

    for (size_t i = start; i < strlen(str); i++) {
        if (isdigit((unsigned char)str[i])) {
            has_digit = 1;
        }
        else if (str[i] == '.') {
            if (has_dot || has_exp) return 0;
            has_dot = 1;
        }
        else if (str[i] == 'e' || str[i] == 'E') {
            if (has_exp || !has_digit) return 0;
            has_exp = 1;
            has_digit = 0;
        }
        else if ((str[i] == '+' || str[i] == '-') && (str[i - 1] == 'e' || str[i - 1] == 'E')) {
            continue;
        }
        else {
            return 0;
        }
    }

    return has_digit;
}

const char* opTypeToString(OpNodeKinds kind) {
    switch (kind) {
    case OP_SUM: return "+";
    case OP_SUB: return "-";
    case OP_MUL: return "*";
    case OP_DIV: return "/";
    case OP_REM: return "%";
        // ... остальные операции
    default: return "UNKNOWN_OP";
    }
}

OpNode* copyOpNode(const OpNode* src) {
    if (!src) return NULL;

    OpNode* copy = calloc(1, sizeof(OpNode));
    memcpy(copy, src, sizeof(OpNode));

    if ((src->kind == OP_PLACE || src->kind == OP_CONST_STRING) && src->payload.string) {
        copy->payload.string = strdup(src->payload.string);
    }

    if (src->argsCount > 0 && src->args) {
        copy->args = malloc(src->argsCount * sizeof(OpNode*));
        copy->argsCount = src->argsCount;
        for (int i = 0; i < src->argsCount; i++) {
            copy->args[i] = copyOpNode(src->args[i]);
        }
    }

    return copy;
}

OpNode* buildOpTree(AstNode* astNode) {
    if (!astNode) return NULL;

    OpNode* opNode = calloc(1, sizeof(OpNode));
    const char* nodeName = astNode->nodeName;

    if (strchr(nodeName, '=') && strcmp(nodeName, "!=") != 0 && strcmp(nodeName, "==") != 0) {
        if (strcmp(nodeName, "=") == 0) {
            // Левая часть - OP_PLACE
            OpNode* target = calloc(1, sizeof(OpNode));
            target->kind = OP_PLACE;
            target->payload.string = strdup(astNode->children[0]->children[0]->nodeName);

            // Правая часть - обычное выражение (с READ)
            OpNode* value = buildOpTree(astNode->children[1]);

            opNode->kind = OP_WRITE;
            opNode->argsCount = 2;
            opNode->args = malloc(2 * sizeof(OpNode*));
            opNode->args[0] = target;
            opNode->args[1] = value;
        }
        else {
            opNode->kind = OP_WRITE;
            opNode->argsCount = 2;
            opNode->args = malloc(2 * sizeof(OpNode*));

            OpNode* target = calloc(1, sizeof(OpNode));
            target->kind = OP_PLACE;
            target->payload.string = strdup(astNode->children[0]->children[0]->nodeName);

            OpNode* operation = calloc(1, sizeof(OpNode));

            if (strcmp(nodeName, "+=") == 0)       operation->kind = OP_SUM;
            else if (strcmp(nodeName, "-=") == 0)  operation->kind = OP_SUB;
            else if (strcmp(nodeName, "*=") == 0)  operation->kind = OP_MUL;
            else if (strcmp(nodeName, "/=") == 0)  operation->kind = OP_DIV;
            else if (strcmp(nodeName, "%=") == 0)  operation->kind = OP_REM;

            operation->argsCount = 2;
            operation->args = malloc(2 * sizeof(OpNode*));

            OpNode* target_copy = buildOpTree(astNode->children[0]); // OP_READ

            operation->args[0] = target_copy;
            operation->args[1] = buildOpTree(astNode->children[1]);

            //opNode->args[0] = target;
            //opNode->args[1] = operation;
            target_copy->payload.string = strdup(target->payload.string);

            operation->args[0] = target_copy;
            operation->args[1] = buildOpTree(astNode->children[1]);

            opNode->args[0] = target;    
            opNode->args[1] = operation;  
        }
    }
    else if (strcmp(nodeName, "Call") == 0) {
        opNode->kind = OP_CALL;
        opNode->argsCount = astNode->children[0]->childrenCount;
        opNode->args = malloc(opNode->argsCount * sizeof(OpNode*));

        for (int i = 0; i < astNode->children[0]->childrenCount; i++) {
            opNode->args[i] = buildOpTree(astNode->children[0]->children[i]);
        }
    }
    else if (strcmp(nodeName, "Identifier") == 0) {
        opNode->kind = OP_READ;
        opNode->argsCount = 1;
        opNode->args = malloc(sizeof(OpNode*));

        // Аргумент READ - OP_PLACE с именем переменной
        OpNode* place = calloc(1, sizeof(OpNode));
        place->kind = OP_PLACE;
        place->payload.string = strdup(astNode->children[0]->nodeName);
        opNode->args[0] = place;
    }
    else if ((strcmp(nodeName, "Expression") == 0 || strcmp(nodeName, "Init_List") == 0) && astNode->childrenCount > 0) {
        return buildOpTree(astNode->children[0]);
    }
    else if (is_integer(nodeName) || is_float(nodeName)) {
        if (strchr(nodeName, '.') || strchr(nodeName, 'e') || strchr(nodeName, 'E')) {
            opNode->kind = OP_CONST_FLOAT;
            opNode->payload.real = atof(nodeName);
        }
        else {
            opNode->kind = OP_CONST_INT;
            opNode->payload.integer = atoi(nodeName);
        }
    }
    else if (strcmp(nodeName, "++") == 0 || strcmp(nodeName, "--") == 0) {
        opNode->kind = OP_WRITE;
        opNode->argsCount = 2;
        opNode->args = malloc(2 * sizeof(OpNode*));

        OpNode* target = calloc(1, sizeof(OpNode));  // buildOpTree(astNode->children[0]);
        OpNode* operation = calloc(1, sizeof(OpNode));

        target->kind = OP_PLACE;
        target->payload.string = strdup(astNode->children[0]->children[0]->nodeName);

        operation->kind = (strcmp(nodeName, "++") == 0) ? OP_SUM : OP_SUB;
        operation->argsCount = 2;
        operation->args = malloc(2 * sizeof(OpNode*));

        OpNode* target_copy = calloc(1, sizeof(OpNode));
        memcpy(target_copy, target, sizeof(OpNode));
        target_copy->payload.string = strdup(target->payload.string);

        operation->args[0] = target_copy;
        operation->args[1] = calloc(1, sizeof(OpNode));
        operation->args[1]->kind = OP_CONST_INT;
        operation->args[1]->payload.integer = 1;

        opNode->args[0] = target;
        opNode->args[1] = operation;
    }
    else if (strcmp(nodeName, "ReturnStatement") == 0) {
        opNode->kind = OP_RETURN;
        if (astNode->childrenCount > 0) {
            opNode->argsCount = 1;
            opNode->args = malloc(sizeof(OpNode*));
            opNode->args[0] = buildOpTree(astNode->children[0]);
        }
    }
    else{
        if (strcmp(nodeName, "!=") == 0)        opNode->kind = OP_CMP_NEQ;
        else if (strcmp(nodeName, "==") == 0)   opNode->kind = OP_CMP_EQ;
        else if (strcmp(nodeName, "+") == 0)    opNode->kind = OP_SUM;
        else if (strcmp(nodeName, "/") == 0)    opNode->kind = OP_DIV;
        else if (strcmp(nodeName, "<") == 0)    opNode->kind = OP_CMP_LT;
        else if (strcmp(nodeName, ">") == 0)    opNode->kind = OP_CMP_GT;
        else if (strcmp(nodeName, "<=") == 0)    opNode->kind = OP_CMP_LT_EQ;
        else if (strcmp(nodeName, ">=") == 0)    opNode->kind = OP_CMP_GT_EQ;
        else if (strcmp(nodeName, "-") == 0)    opNode->kind = OP_SUB;
        else if (strcmp(nodeName, "*") == 0)    opNode->kind = OP_MUL;

        if (opNode->kind && astNode->childrenCount > 1) {
            AstNode* left = astNode->children[0];
            AstNode* right = astNode->children[1];
            opNode->argsCount = 2;
            opNode->args = malloc(2 * sizeof(OpNode*));
            opNode->args[0] = buildOpTree(left);
            opNode->args[1] = buildOpTree(right);
        }
        else {
            opNode->kind = OP_CONST_STRING;
            char* str_content = strdup(nodeName);
            opNode->payload.string = str_content;
        }
    }
    return opNode;
}

char* opTreeToString(OpNode* node) {
    if (!node) return strdup("NULL");

    switch (node->kind) {
    case OP_READ: {
        char* arg = opTreeToString(node->args[0]);
        char* res = malloc(strlen(arg) + 10);
        sprintf(res, "READ(%s)", arg);
        free(arg);
        return res;
    }
    case OP_WRITE: {
        char* target = opTreeToString(node->args[0]);
        char* value = opTreeToString(node->args[1]);
        char* res = malloc(strlen(target) + strlen(value) + 10);
        sprintf(res, "WRITE(%s, %s)", target, value);
        free(target);
        free(value);
        return res;
    }
    case OP_READ_INDEX: {
        char* arr = opTreeToString(node->args[0]);
        char* index = opTreeToString(node->args[1]);
        char* res = malloc(strlen(arr) + strlen(index) + 10);
        sprintf(res, "%s[%s]", arr, index);
        free(arr);
        free(index);
        return res;
    }
    case OP_WRITE_INDEX: {
        char* arr = opTreeToString(node->args[0]);
        char* index = opTreeToString(node->args[1]);
        char* value = opTreeToString(node->args[2]);
        char* res = malloc(strlen(arr) + strlen(index) + strlen(value) + 20);
        sprintf(res, "%s[%s] = %s", arr, index, value);
        free(arr);
        free(index);
        free(value);
        return res;
    }
    case OP_SUM: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s + %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_SUB: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s - %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_NEG: {
        char* arg = opTreeToString(node->args[0]);
        char* res = malloc(strlen(arg) + 5);
        sprintf(res, "(-%s)", arg);
        free(arg);
        return res;
    }
    case OP_REM: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s %% %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_DIV: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s / %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_MUL: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s * %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_GT: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s > %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_LT: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s < %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_LT_EQ: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s <= %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_GT_EQ: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s >= %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_EQ: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s == %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_CMP_NEQ: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s != %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BITS_OR: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s | %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BITS_AND: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s & %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BITS_XOR: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s ^ %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BITS_INV: {
        char* arg = opTreeToString(node->args[0]);
        char* res = malloc(strlen(arg) + 5);
        sprintf(res, "(~%s)", arg);
        free(arg);
        return res;
    }
    case OP_BITS_SHL: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s << %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BITS_SHR: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s >> %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BOOL_AND: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s && %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BOOL_OR: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s || %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BOOL_XOR: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "(%s ^^ %s)", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_BOOL_NOT: {
        char* arg = opTreeToString(node->args[0]);
        char* res = malloc(strlen(arg) + 5);
        sprintf(res, "(!%s)", arg);
        free(arg);
        return res;
    }
    case OP_CALL: {
        char* func = opTreeToString(node->args[0]);
        char args[1024] = "";
        for (int i = 1; i < node->argsCount; i++) {
            char* arg = opTreeToString(node->args[i]);
            strcat(args, arg);
            if (i != node->argsCount - 1) strcat(args, ", ");
            free(arg);
        }
        char* res = malloc(strlen(func) + strlen(args) + 10);
        sprintf(res, "CALL(%s(%s))", func, args);
        free(func);
        return res;
    }
    case OP_PLACE:
        return strdup(node->payload.string);
    case OP_CONST_INT: {
        char* res = malloc(20);
        sprintf(res, "%d", node->payload.integer);
        return res;
    }
    case OP_CONST_FLOAT: {
        char* res = malloc(20);
        sprintf(res, "%.2f", node->payload.real);
        return res;
    }
    case OP_CONST_STRING: {
        char* res = malloc(strlen(node->payload.string) + 3);
        sprintf(res, "\"%s\"", node->payload.string);
        return res;
    }
    case OP_RETURN: {
        char* arg = node->argsCount > 0 ? opTreeToString(node->args[0]) : strdup("");
        char* res = malloc(strlen(arg) + 10);
        sprintf(res, "RETURN %s", arg);
        free(arg);
        return res;
    }
    case OP_ADD_ASSIGN: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "%s += %s", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_DIV_ASSIGN: {
        char* left = opTreeToString(node->args[0]);
        char* right = opTreeToString(node->args[1]);
        char* res = malloc(strlen(left) + strlen(right) + 10);
        sprintf(res, "%s /= %s", left, right);
        free(left);
        free(right);
        return res;
    }
    case OP_UNKNOWN:
    default: {
        char* res = malloc(20);
        sprintf(res, "UNKNOWN_OP_%d", node->kind);
        return res;
    }
    }
}

void freeOpTree(OpNode* node) {
    if (!node) return;

    if (node->args) {
        for (int i = 0; i < node->argsCount; i++) {
            if (node->args[i]) {
                freeOpTree(node->args[i]);
            }
        }
        free(node->args);
    }

    if ((node->kind == OP_PLACE || node->kind == OP_CONST_STRING) && node->payload.string) {
        free(node->payload.string);
    }

    free(node);
}