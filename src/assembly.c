#include "assembly.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StackVar {
    char *name;
    int offset;
    struct StackVar *next;
} StackVar;

typedef struct GlobalVar {
    char *name;
    struct GlobalVar *next;
} GlobalVar;

static StackVar *stackHead = NULL;
static GlobalVar *globalHead = NULL;
static int currentStackOffset = 0;


static void registerGlobal(const char *name) {
    for (GlobalVar *curr = globalHead; curr != NULL; curr = curr->next) {
        if (strcmp(curr->name, name) == 0) return;
    }
    GlobalVar *g = (GlobalVar *)malloc(sizeof(GlobalVar));
    g->name = strdup(name);
    g->next = globalHead;
    globalHead = g;
}

static int isGlobalVar(const char *name) {
    for (GlobalVar *curr = globalHead; curr != NULL; curr = curr->next) {
        if (strcmp(curr->name, name) == 0) return 1;
    }
    return 0;
}

static void collectGlobalsAndEmit(ASTNode *node, FILE *out) {
    if (node == NULL || node->symbolData == NULL) return;

    if (node->symbolData->symbolType == varDeclarationList) {
        collectGlobalsAndEmit(node->left, out);
        collectGlobalsAndEmit(node->right, out);
    } else if (node->symbolData->symbolType == varDeclaration) {
        ASTNode *curr = node;
        while (curr != NULL && curr->symbolData->symbolType == varDeclaration) {
            if (curr->left && curr->left->symbolData) {
                char *varName = curr->left->symbolData->id;
                registerGlobal(varName);
                fprintf(out, "    .comm %s, 8, 8\n", varName);
            }
            curr = curr->right;
        }
    }
}

static void clearStackVars(void) {
    while (stackHead != NULL) {
        StackVar *tmp = stackHead;
        stackHead = stackHead->next;
        free(tmp->name);
        free(tmp);
    }
    currentStackOffset = 0;
}

static int getOrAssignOffset(Symbol *s) {
    if (s == NULL || s->id == NULL || strlen(s->id) == 0) return 0;
    if (s->symbolType == intLiteral || s->symbolType == boolLiteral ||
        s->symbolType == floatLiteral || s->symbolType == blankNode ||
        s->symbolType == function || s->symbolType == functionInvoke) {
        return 0;
    }
    if (isGlobalVar(s->id)) return 0;

    for (StackVar *curr = stackHead; curr != NULL; curr = curr->next) {
        if (strcmp(curr->name, s->id) == 0) {
            return curr->offset;
        }
    }

    currentStackOffset -= 8;
    StackVar *sv = (StackVar *)malloc(sizeof(StackVar));
    sv->name = strdup(s->id);
    sv->offset = currentStackOffset;
    sv->next = stackHead;
    stackHead = sv;
    return sv->offset;
}

static int buildFunctionStackFrame(CodinterInstruction *funcBegin) {
    clearStackVars();
    Symbol *funcSym = funcBegin->arg1;

    if (funcSym != NULL && funcSym->paramsNode != NULL) {
        ASTNode *currParam = funcSym->paramsNode;
        while (currParam != NULL && currParam->symbolData->symbolType == formalParameterList) {
            if (currParam->left && currParam->left->symbolData) {
                getOrAssignOffset(currParam->left->symbolData);
            }
            currParam = currParam->right;
        }
    }

    for (CodinterInstruction *curr = funcBegin->next;
         curr != NULL && curr->op != Codinter_FUNC_END;
         curr = curr->next) {
        if (curr->op != Codinter_LABEL && curr->op != Codinter_GOTO &&
            curr->op != Codinter_IF_FALSE) {
            getOrAssignOffset(curr->arg1);
            getOrAssignOffset(curr->arg2);
            getOrAssignOffset(curr->result);
        } else if (curr->op == Codinter_IF_FALSE) {
            getOrAssignOffset(curr->arg1);
        }
    }

    /* Alinear el tamaño del stack a múltiplos de 16 bytes (ABI x86-64) */
    int frameSize = -currentStackOffset;
    if (frameSize % 16 != 0) {
        frameSize += 8;
    }
    return frameSize;
}


static void loadOperand(Symbol *s, const char *reg, FILE *out) {
    if (s == NULL) return;

    if (s->symbolType == intLiteral) {
        fprintf(out, "    movq $%d, %s\n", s->value.intValue, reg);
    } else if (s->symbolType == boolLiteral) {
        fprintf(out, "    movq $%d, %s\n", s->value.boolValue ? 1 : 0, reg);
    } else if (isGlobalVar(s->id)) {
        fprintf(out, "    movq %s(%%rip), %s\n", s->id, reg);
    } else {
        int offset = getOrAssignOffset(s);
        fprintf(out, "    movq %d(%%rbp), %s\n", offset, reg);
    }
}

static void storeOperand(const char *reg, Symbol *dest, FILE *out) {
    if (dest == NULL) return;

    if (isGlobalVar(dest->id)) {
        fprintf(out, "    movq %s, %s(%%rip)\n", reg, dest->id);
    } else {
        int offset = getOrAssignOffset(dest);
        fprintf(out, "    movq %s, %d(%%rbp)\n", reg, offset);
    }
}

void generateAssembly(ASTNode *astRoot, CodinterInstruction *codinterHead, FILE *out) {
    if (out == NULL) return;

    const char *argRegs[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    Symbol *pendingParams[16];
    int paramCount = 0;

    fprintf(out, "    .text\n");

    /* 1. Declarar variables globales si existen */
    if (astRoot != NULL && astRoot->left != NULL) {
        collectGlobalsAndEmit(astRoot->left, out);
    }

    /* 2. Traducir cada instrucción de código intermedio */
    for (CodinterInstruction *curr = codinterHead; curr != NULL; curr = curr->next) {
        switch (curr->op) {
            case Codinter_FUNC_BEGIN: {
                int frameSize = buildFunctionStackFrame(curr);
                fprintf(out, "\n    .globl %s\n", curr->arg1->id);
                fprintf(out, "%s:\n", curr->arg1->id);
                fprintf(out, "    pushq %%rbp\n");
                fprintf(out, "    movq %%rsp, %%rbp\n");
                if (frameSize > 0) {
                    fprintf(out, "    subq $%d, %%rsp\n", frameSize);
                }

                if (curr->arg1->paramsNode != NULL) {
                    ASTNode *p = curr->arg1->paramsNode;
                    int idx = 0;
                    while (p != NULL && p->symbolData->symbolType == formalParameterList && idx < 6) {
                        if (p->left && p->left->symbolData) {
                            storeOperand(argRegs[idx], p->left->symbolData, out);
                            idx++;
                        }
                        p = p->right;
                    }
                }
                break;
            }

            case Codinter_FUNC_END: {
                fprintf(out, "    movq $0, %%rax\n");
                fprintf(out, "    leave\n");
                fprintf(out, "    ret\n");
                break;
            }

            case Codinter_ASSIGN: {
                loadOperand(curr->arg1, "%r10", out);
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_ADD: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    addq %%r11, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_SUB: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    subq %%r11, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_MUL: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    imulq %%r11, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_DIV: {
                loadOperand(curr->arg1, "%rax", out);
                fprintf(out, "    cqto\n");
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    idivq %%r11\n");
                storeOperand("%rax", curr->result, out);
                break;
            }

            case Codinter_MOD: {
                loadOperand(curr->arg1, "%rax", out);
                fprintf(out, "    cqto\n");
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    idivq %%r11\n");
                storeOperand("%rdx", curr->result, out);
                break;
            }

            case Codinter_NEG: {
                loadOperand(curr->arg1, "%r10", out);
                fprintf(out, "    negq %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_NOT: {
                loadOperand(curr->arg1, "%r10", out);
                fprintf(out, "    xorq $1, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_AND: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    andq %%r11, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_OR: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    orq %%r11, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_LT:
            case Codinter_GT:
            case Codinter_EQ: {
                loadOperand(curr->arg1, "%r10", out);
                loadOperand(curr->arg2, "%r11", out);
                fprintf(out, "    cmpq %%r11, %%r10\n");
                if (curr->op == Codinter_LT)      fprintf(out, "    setl %%al\n");
                else if (curr->op == Codinter_GT) fprintf(out, "    setg %%al\n");
                else                              fprintf(out, "    sete %%al\n");
                fprintf(out, "    movzbq %%al, %%r10\n");
                storeOperand("%r10", curr->result, out);
                break;
            }

            case Codinter_LABEL: {
                fprintf(out, ".%s:\n", curr->result->id);
                break;
            }

            case Codinter_GOTO: {
                fprintf(out, "    jmp .%s\n", curr->result->id);
                break;
            }

            case Codinter_IF_FALSE: {
                loadOperand(curr->arg1, "%r10", out);
                fprintf(out, "    cmpq $0, %%r10\n");
                fprintf(out, "    je .%s\n", curr->result->id);
                break;
            }

            case Codinter_PARAM: {
                if (paramCount < 16) {
                    pendingParams[paramCount++] = curr->arg1;
                }
                break;
            }

            case Codinter_CALL: {
                for (int i = 0; i < paramCount && i < 6; i++) {
                    loadOperand(pendingParams[i], argRegs[i], out);
                }
                paramCount = 0;
                fprintf(out, "    call %s\n", curr->arg1->id);
                if (curr->result != NULL) {
                    storeOperand("%rax", curr->result, out);
                }
                break;
            }

            case Codinter_RETURN: {
                if (curr->arg1 != NULL) {
                    loadOperand(curr->arg1, "%rax", out);
                } else {
                    fprintf(out, "    movq $0, %%rax\n");
                }
                fprintf(out, "    leave\n");
                fprintf(out, "    ret\n");
                break;
            }
        }
    }

    fprintf(out, "\n    .section .note.GNU-stack,\"\",@progbits\n");
    fflush(out);
}