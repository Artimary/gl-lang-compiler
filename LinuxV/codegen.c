#include "codegen.h"
#include "cfg.h"
#include "OpTree.h"
#include "treeStructure.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

// Безопасный доступ к операндам
static const char* safe_operand(const char* op) {
    static const char* empty = "";
    if (!op || (uintptr_t)op < 0x1000) {
        return empty;
    }
    return op;
}

// Безопасное дублирование строк
static char* safe_strdup(const char* str) {
    static const char* empty = "";
    char* result = strdup(str ? str : empty);
    return result ? result : (char*)empty;
}

// Основная функция генерации образа программы
ProgramImage* generateProgramImage(ProgramUnit* unit) {
    ProgramImage* img = malloc(sizeof(ProgramImage));
    if (!img) return NULL;

    img->func_count = unit->funcsCount;
    img->functions = malloc(sizeof(FunctionCode) * img->func_count);
    if (!img->functions) {
        free(img);
        return NULL;
    }

    img->global_count = 0;
    img->globals = NULL;

    for (int i = 0; i < unit->funcsCount; i++) {
        img->functions[i] = generateFunctionCode(&unit->funcs[i], img);
    }

    return img;
}

// Вспомогательная функция для топологической сортировки
static void topologicalSortUtil(CfgNode* node, bool* visited, CfgNode** stack, int* index, int capacity) {
    visited[node->id] = true;
    if (node->nextNode && !visited[node->nextNode->id]) {
        topologicalSortUtil(node->nextNode, visited, stack, index, capacity);
    }
    if (node->condNode && !visited[node->condNode->id]) {
        topologicalSortUtil(node->condNode, visited, stack, index, capacity);
    }
    stack[(*index)++] = node;
}

// Топологическая сортировка CFG
static CfgNode** topologicalSort(CfgInstance* cfg, int* size) {
    bool* visited = calloc(cfg->capacity, sizeof(bool));
    CfgNode** stack = malloc(cfg->capacity * sizeof(CfgNode*));
    int index = 0;

    if (!visited || !stack) {
        free(visited);
        free(stack);
        *size = 0;
        return NULL;
    }

    topologicalSortUtil(cfg->nodes[0], visited, stack, &index, cfg->capacity);
    free(visited);
    *size = index;
    return stack;
}

// Генерация пролога функции
static void generatePrologue(FunctionCode* fc, int stack_size) {
    void* new_ptr = realloc(fc->instructions, (fc->instr_count + 4) * sizeof(Instruction));
    if (!new_ptr) {
        fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в generatePrologue\n");
        return;
    }
    fc->instructions = new_ptr;
    Instruction* instr = fc->instructions + fc->instr_count;

    instr[0].mnemonic = strdup("stp");
    instr[0].operands[0] = strdup("x29");
    instr[0].operands[1] = strdup("x30");
    instr[0].operands[2] = strdup("[sp, #-16]!");
    instr[0].size = 4;

    instr[1].mnemonic = strdup("mov");
    instr[1].operands[0] = strdup("x29");
    instr[1].operands[1] = strdup("sp");
    instr[1].size = 4;

    char imm[16];
    snprintf(imm, 16, "#%d", stack_size);
    instr[2].mnemonic = strdup("sub");
    instr[2].operands[0] = strdup("sp");
    instr[2].operands[1] = strdup("sp");
    instr[2].operands[2] = strdup(imm);
    instr[2].size = 4;

    instr[3].mnemonic = strdup("stp");
    instr[3].operands[0] = strdup("x19");
    instr[3].operands[1] = strdup("x20");
    instr[3].operands[2] = strdup("[sp, #-16]!");
    instr[3].size = 4;

    fc->instr_count += 4;
}

// Генерация эпилога функции
static void generateEpilogue(FunctionCode* fc, int stack_size) {
    void* new_ptr = realloc(fc->instructions, (fc->instr_count + 4) * sizeof(Instruction));
    if (!new_ptr) {
        fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в generateEpilogue\n");
        return;
    }
    fc->instructions = new_ptr;
    Instruction* instr = fc->instructions + fc->instr_count;

    instr[0].mnemonic = strdup("ldp");
    instr[0].operands[0] = strdup("x19");
    instr[0].operands[1] = strdup("x20");
    instr[0].operands[2] = strdup("[sp], #16");
    instr[0].size = 4;

    char imm[16];
    snprintf(imm, 16, "#%d", stack_size);
    instr[1].mnemonic = strdup("add");
    instr[1].operands[0] = strdup("sp");
    instr[1].operands[1] = strdup("sp");
    instr[1].operands[2] = strdup(imm);
    instr[1].size = 4;

    instr[2].mnemonic = strdup("ldp");
    instr[2].operands[0] = strdup("x29");
    instr[2].operands[1] = strdup("x30");
    instr[2].operands[2] = strdup("[sp], #16");
    instr[2].size = 4;

    instr[3].mnemonic = strdup("ret");
    instr[3].size = 4;

    fc->instr_count += 4;
}

// Проверка локальной переменной
static bool isLocalVariable(FunctionCode* fc, const char* varName) {
    for (int i = 0; i < fc->locals_count; i++) {
        if (strcmp(fc->locals[i].name, varName) == 0) {
            return true;
        }
    }
    return false;
}

// Расчет размера стека
static int calculateStackSize(FuncSignatureN* sig) {
    int size = 32;
    if (sig) {
        size += sig->argsCount * 8;
    }
    size = (size + 15) & ~15;
    return size;
}

// Генерация меток
static void generateLabels(CfgInstance* cfg, const char* funcName) {

    for (int i = 0; i < cfg->count; i++) {
        CfgNode* node = cfg->nodes[i];
        if (node->nodeName) free(node->nodeName);
        node->nodeName = malloc(32);
        if (!node->nodeName) {
            // Обработка ошибки: освобождение ресурсов, возврат NULL и т.д.
            return NULL; // или аналогичное действие
        }
        snprintf(node->nodeName, 32, ".L%s_%d", funcName, node->id);
    }
}

// Получение мнемоники
static const char* getMnemonic(OpNodeKinds kind) {
    switch (kind) {
    case OP_SUM: return "add";
    case OP_SUB: return "sub";
    case OP_MUL: return "mul";
    case OP_DIV: return "sdiv";
    case OP_REM: return "udiv";
    case OP_NEG: return "neg";
    case OP_WRITE: return "str";
    case OP_READ: return "ldr";
    case OP_READ_INDEX: return "ldr";
    case OP_WRITE_INDEX: return "str";
    case OP_CMP_GT: return "cmp";
    case OP_CMP_GT_EQ: return "cmp";
    case OP_CMP_EQ: return "cmp";
    case OP_CMP_NEQ: return "cmp";
    case OP_CMP_LT: return "cmp";
    case OP_CMP_LT_EQ: return "cmp";
    case OP_BITS_OR: return "orr";
    case OP_BITS_AND: return "and";
    case OP_BITS_XOR: return "eor";
    case OP_BITS_INV: return "mvn";
    case OP_BITS_SHL: return "lsl";
    case OP_BITS_SHR: return "lsr";
    case OP_BOOL_AND: return "and";
    case OP_BOOL_OR: return "orr";
    case OP_BOOL_NOT: return "eor";
    case OP_BOOL_XOR: return "eor";
    case OP_ADD_ASSIGN: return "add";
    case OP_SUB_ASSIGN: return "sub";
    case OP_MUL_ASSIGN: return "mul";
    case OP_DIV_ASSIGN: return "sdiv";
    case OP_REM_ASSIGN: return "udiv";
    case OP_INC: return "add";
    case OP_DEC: return "sub";
    case OP_RETURN: return "ret";
    case OP_CALL: return "bl";
    case OP_CONST_INT:
    case OP_CONST_FLOAT:
    case OP_CONST_STRING: return "mov";
    default: return "nop";
    }
}

// Рекурсивная обработка узлов
static const char* processOpNode(OpNode* op, FunctionCode* fc, uintptr_t* address, int* reg_counter) {
    if (!op) return NULL;

    char reg[4];
    snprintf(reg, 4, "X%d", *reg_counter);
    (*reg_counter)++;

    void* new_ptr = realloc(fc->instructions, fc->instr_count * sizeof(Instruction));
    if (!new_ptr) {
        fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в processOpNode\n");
        return NULL;
    }
    fc->instructions = new_ptr;
    Instruction* instr = &fc->instructions[fc->instr_count++];
    instr->mnemonic = NULL;
    instr->operands[0] = NULL;
    instr->operands[1] = NULL;
    instr->operands[2] = NULL;
    instr->address = *address;
    instr->size = 4;

    switch (op->kind) {
    case OP_PLACE: {
        instr->mnemonic = strdup("ldr");
        instr->operands[0] = strdup(reg);
        instr->operands[1] = strdup("=");
        instr->operands[2] = safe_strdup(op->payload.string);
        *address += 4;
        break;
    }

    case OP_CONST_INT: {
        instr->mnemonic = strdup("mov");
        instr->operands[0] = strdup(reg);
        char imm[16];
        snprintf(imm, 16, "#%d", op->payload.integer);
        instr->operands[1] = strdup(imm);
        *address += 4;
        break;
    }

    case OP_SUM:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV: {
        const char* left_reg = processOpNode(op->args[0], fc, address, reg_counter);
        const char* right_reg = processOpNode(op->args[1], fc, address, reg_counter);
        if (!left_reg || !right_reg) {
            free((void*)left_reg);
            free((void*)right_reg);
            return NULL;
        }
        instr->mnemonic = strdup(op->kind == OP_SUM ? "add" : op->kind == OP_SUB ? "sub" :
            op->kind == OP_MUL ? "mul" : "sdiv");
        instr->operands[0] = strdup(reg);
        instr->operands[1] = strdup(left_reg);
        instr->operands[2] = strdup(right_reg);
        *address += 4;
        free((void*)left_reg);
        free((void*)right_reg);
        break;
    }

    default:
        (*reg_counter)--;
        fc->instr_count--;
        return NULL;
    }

    return strdup(reg);
}

// Перевод базового блока
static void translateBasicBlock(CfgNode* node, FunctionCode* fc, uintptr_t* address) {
    OpNode* op = node->optree;
    if (!op) return;

    int reg_counter = 0;

    printf("translateBasicBlock: fc->instructions = %p, fc->instr_count = %d\n",
        (void*)fc->instructions, fc->instr_count);

    if (op->kind == OP_WRITE) {
        const char* result_reg = processOpNode(op->args[1], fc, address, &reg_counter);
        if (result_reg) {
            void* new_ptr = realloc(fc->instructions, fc->instr_count * sizeof(Instruction));
            if (!new_ptr) {
                fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в translateBasicBlock (OP_WRITE)\n");
                free((void*)result_reg);
                return;
            }
            fc->instructions = new_ptr;
            Instruction* str = &fc->instructions[fc->instr_count++];
            str->mnemonic = strdup("str");
            str->operands[0] = strdup(result_reg);
            str->operands[1] = strdup("=");
            str->operands[2] = strdup(op->args[0]->payload.string);
            str->address = *address;
            *address += 4;
            str->size = 4;
            free((void*)result_reg);
        }
    }
    else if (op->kind == OP_RETURN) {
        if (op->argsCount > 0 && op->args && op->args[0]) {
            const char* result_reg = processOpNode(op->args[0], fc, address, &reg_counter);
            if (result_reg) {
                void* new_ptr = realloc(fc->instructions, fc->instr_count * sizeof(Instruction));
                if (!new_ptr) {
                    fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в translateBasicBlock (OP_RETURN MOV)\n");
                    free((void*)result_reg);
                    return;
                }
                fc->instructions = new_ptr;
                Instruction* mov = &fc->instructions[fc->instr_count++];
                mov->mnemonic = strdup("mov");
                mov->operands[0] = strdup("x0");
                mov->operands[1] = strdup(result_reg);
                mov->address = *address;
                *address += 4;
                mov->size = 4;
                free((void*)result_reg);
            }
        }
        void* new_ptr = realloc(fc->instructions, fc->instr_count * sizeof(Instruction));
        if (!new_ptr) {
            fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в translateBasicBlock (OP_RETURN RET)\n");
            return;
        }
        fc->instructions = new_ptr;
        Instruction* ret = &fc->instructions[fc->instr_count++];
        ret->mnemonic = strdup("ret");
        ret->address = *address;
        *address += 4;
        ret->size = 4;
    }
    else {
        const char* result_reg = processOpNode(op, fc, address, &reg_counter);
        if (result_reg) free((void*)result_reg);
    }
}

// Генерация переходов
static void emitBranching(CfgNode* node, FunctionCode* fc) {
    if (node->nextNode && node->optree && node->optree->kind != OP_RETURN) {
        printf("emitBranching: fc->instructions = %p, fc->instr_count = %d\n",
            (void*)fc->instructions, fc->instr_count);

        if (fc->instr_count < 0) {
            fprintf(stderr, "Ошибка: fc->instr_count отрицательный (%d) в emitBranching\n", fc->instr_count);
            return;
        }

        void* new_ptr = realloc(fc->instructions, (fc->instr_count + 1) * sizeof(Instruction));
        if (!new_ptr) {
            fprintf(stderr, "Ошибка: не удалось выделить память для fc->instructions в emitBranching\n");
            return;
        }
        fc->instructions = new_ptr;

        memset(&fc->instructions[fc->instr_count], 0, sizeof(Instruction));

        Instruction* instr = &fc->instructions[fc->instr_count++];
        instr->mnemonic = strdup("b");
        const char* target = node->nextNode ? node->nextNode->nodeName : ".Lnext";
        instr->operands[0] = strdup(target ? target : ".Lnext");
        instr->address = fc->instructions[fc->instr_count - 2].address + 4;
        instr->size = 4;
    }
}

// Генерация кода функции
FunctionCode generateFunctionCode(FuncDefN* func, ProgramImage* img) {
    FunctionCode fc = {
        .name = func->name ? strdup(func->name) : strdup("unknown"),
        .instr_count = 0,
        .instructions = NULL,
        .locals_count = 0,
        .locals = NULL
    };

    generateLabels(func->cfg, func->name);
    int stack_size = calculateStackSize(func->signature);
    generatePrologue(&fc, stack_size);

    int sorted_size;
    CfgNode** sorted = topologicalSort(func->cfg, &sorted_size);
    if (!sorted) {
        free(fc.name);
        return fc;
    }

    uintptr_t current_address = 0x400000 + fc.instr_count * 4;
    for (int i = sorted_size - 1; i >= 0; i--) {
        CfgNode* node = sorted[i];
        translateBasicBlock(node, &fc, &current_address);
        emitBranching(node, &fc);
    }

    generateEpilogue(&fc, stack_size);
    free(sorted);
    return fc;
}

// Освобождение памяти
void freeProgramImage(ProgramImage* img) {
    if (!img) return;

    for (int i = 0; i < img->func_count; i++) {
        FunctionCode* fc = &img->functions[i];
        for (int j = 0; j < fc->instr_count; j++) {
            free(fc->instructions[j].mnemonic);
            for (int k = 0; k < 3; k++) {
                free(fc->instructions[j].operands[k]);
            }
        }
        free(fc->instructions);
        free(fc->name);
    }
    free(img->functions);
    free(img);
}