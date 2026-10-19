#include "tac.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TACInstruction *tacHead = NULL;
static TACInstruction *tacTail = NULL;
static int tempCounter = 0;
static int labelCounter = 0;

void initTACGenerator(void) {
    tacHead = NULL;
    tacTail = NULL;
    tempCounter = 0;
    labelCounter = 0;
}

TACInstruction *getTACListHead(void) {
    return tacHead;
}

// Emite una instrucción al final de la lista lineal
static TACInstruction *emit(TACOp op, Symbol *arg1, Symbol *arg2, Symbol *result) {
    TACInstruction *inst = (TACInstruction *)malloc(sizeof(TACInstruction));
    inst->op = op;
    inst->arg1 = arg1;
    inst->arg2 = arg2;
    inst->result = result;
    inst->next = NULL;

    if (tacHead == NULL) {
        tacHead = inst;
        tacTail = inst;
    } else {
        tacTail->next = inst;
        tacTail = inst;
    }
    return inst;
}

// Genera un temporal nuevo: t0, t1, t2...
static Symbol *newTemp(enum types type) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "t%d", tempCounter++);
    Symbol *temp = makeSymbol(buffer, local_var);
    temp->type = type;
    return temp;
}

// Genera una etiqueta nueva: L0, L1, L2...
static Symbol *newLabel(void) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "L%d", labelCounter++);
    Symbol *label = makeSymbol(buffer, blankNode);
    return label;
}

// Recorre una expresión y devuelve el Symbol* (variable, literal o temporal) con el resultado
Symbol *generateExprTAC(ASTNode *expr) {
    if (expr == NULL || expr->symbolData == NULL) return NULL;

    Symbol *sym = expr->symbolData;

    switch (sym->symbolType) {
        case intLiteral:
        case boolLiteral:
        case floatLiteral: {
            // Opcional: puedes devolver directamente el literal o cargarlo en un temporal (Load_Const)
            Symbol *t = newTemp(sym->type);
            emit(TAC_ASSIGN, sym, NULL, t);
            return t;
        }

        case variable:
        case global_var:
        case local_var:
        case formalParameter:
            return sym;

        case addOp:
        case substractOp:
        case multOp:
        case divOp:
        case modOp:
        case lessOp:
        case greaterOp:
        case equalsOp:
        case andOp:
        case orOp: {
            // Caso especial: resta unaria (-expr) tiene expr->right == NULL en tu parser.y
            if (sym->symbolType == substractOp && expr->right == NULL) {
                Symbol *op1 = generateExprTAC(expr->left);
                Symbol *res = newTemp(op1->type);
                emit(TAC_NEG, op1, NULL, res);
                return res;
            }

            Symbol *leftOp = generateExprTAC(expr->left);
            Symbol *rightOp = generateExprTAC(expr->right);

            enum types resType = tInt;
            if (sym->symbolType == lessOp || sym->symbolType == greaterOp ||
                sym->symbolType == equalsOp || sym->symbolType == andOp ||
                sym->symbolType == orOp) {
                resType = tBoolean;
            }

            Symbol *res = newTemp(resType);
            TACOp opCode;
            switch (sym->symbolType) {
                case addOp:       opCode = TAC_ADD; break;
                case substractOp: opCode = TAC_SUB; break;
                case multOp:      opCode = TAC_MUL; break;
                case divOp:       opCode = TAC_DIV; break;
                case modOp:       opCode = TAC_MOD; break;
                case lessOp:      opCode = TAC_LT;  break;
                case greaterOp:   opCode = TAC_GT;  break;
                case equalsOp:    opCode = TAC_EQ;  break;
                case andOp:       opCode = TAC_AND; break;
                case orOp:        opCode = TAC_OR;  break;
                default:          opCode = TAC_ADD; break;
            }
            emit(opCode, leftOp, rightOp, res);
            return res;
        }

        case negationOp: {
            Symbol *op1 = generateExprTAC(expr->left);
            Symbol *res = newTemp(tBoolean);
            emit(TAC_NOT, op1, NULL, res);
            return res;
        }

        case functionInvoke: {
            // 1. Evaluar argumentos actuales y emitir PARAM
            ASTNode *currParam = expr->left;
            while (currParam != NULL && currParam->symbolData->symbolType == actualParameterList) {
                Symbol *argSym = generateExprTAC(currParam->left);
                emit(TAC_PARAM, argSym, NULL, NULL);
                currParam = currParam->right;
            }
            // 2. Emitir CALL
            if (sym->type != tVoid && sym->type != blank) {
                Symbol *res = newTemp(sym->type);
                emit(TAC_CALL, sym, NULL, res);
                return res;
            } else {
                emit(TAC_CALL, sym, NULL, NULL);
                return NULL;
            }
        }

        default:
            return sym;
    }
}

// Recorre sentencias, bloques y declaraciones del AST
void generateTACFromAST(ASTNode *node) {
    if (node == NULL || node->symbolData == NULL) return;

    Symbol *sym = node->symbolData;

    switch (sym->symbolType) {
        case program:
        case methodDeclarationList:
        case block:
        case statementList:
            generateTACFromAST(node->left);
            generateTACFromAST(node->right);
            break;

        case varDeclarationList:
        case varDeclaration:
            // Las declaraciones de variables ya están en la tabla de símbolos
            break;

        case function: {
            emit(TAC_FUNC_BEGIN, sym, NULL, NULL);
            generateTACFromAST(node->right); // Cuerpo de la función (block)
            emit(TAC_FUNC_END, sym, NULL, NULL);
            break;
        }

        case assignOp: {
            Symbol *rhs = generateExprTAC(node->right);
            Symbol *lhs = node->left->symbolData;
            emit(TAC_ASSIGN, rhs, NULL, lhs);
            break;
        }

        case ifNode: {
            Symbol *cond = generateExprTAC(node->left);
            Symbol *labelFalse = newLabel();

            // Si la condición es falsa, salta a labelFalse
            emit(TAC_IF_FALSE, cond, NULL, labelFalse);

            // Bloque THEN (hijo izquierdo del blankNode)
            ASTNode *ifBody = node->right;
            if (ifBody != NULL) {
                generateTACFromAST(ifBody->left);
            }

            // Verificar si existe rama ELSE (hijo derecho del blankNode)
            if (ifBody != NULL && ifBody->right != NULL) {
                Symbol *labelEnd = newLabel();
                emit(TAC_GOTO, NULL, NULL, labelEnd);
                emit(TAC_LABEL, NULL, NULL, labelFalse);
                // El bloque del elseNode está en ifBody->right->left
                generateTACFromAST(ifBody->right->left);
                emit(TAC_LABEL, NULL, NULL, labelEnd);
            } else {
                emit(TAC_LABEL, NULL, NULL, labelFalse);
            }
            break;
        }

        case whileNode: {
            Symbol *labelStart = newLabel();
            Symbol *labelEnd = newLabel();

            // Etiqueta de inicio antes de evaluar la condición
            emit(TAC_LABEL, NULL, NULL, labelStart);
            Symbol *cond = generateExprTAC(node->left);
            emit(TAC_IF_FALSE, cond, NULL, labelEnd);

            // Cuerpo del while
            generateTACFromAST(node->right);
            emit(TAC_GOTO, NULL, NULL, labelStart);

            // Etiqueta de salida del bucle
            emit(TAC_LABEL, NULL, NULL, labelEnd);
            break;
        }

        case returnNode: {
            if (node->left != NULL) {
                Symbol *retVal = generateExprTAC(node->left);
                emit(TAC_RETURN, retVal, NULL, NULL);
            } else {
                emit(TAC_RETURN, NULL, NULL, NULL);
            }
            break;
        }

        case functionInvoke: {
            generateExprTAC(node);
            break;
        }

        default:
            generateTACFromAST(node->left);
            generateTACFromAST(node->right);
            break;
    }  
}

static void formatOperand(Symbol *s, char *buf, size_t size) {
    if (s == NULL) {
        snprintf(buf, size, "");
        return;
    }
    if (s->symbolType == intLiteral) {
        snprintf(buf, size, "%d", s->value.intValue);
    } else if (s->symbolType == boolLiteral) {
        snprintf(buf, size, "%s", s->value.boolValue ? "true" : "false");
    } else if (s->symbolType == floatLiteral) {
        snprintf(buf, size, "%f", s->value.floatValue);
    } else {
        snprintf(buf, size, "%s", s->id);
    }
}

void printTAC(FILE *out) {
    if (out == NULL) return;
    char a1[64], a2[64], res[64];

    for (TACInstruction *curr = tacHead; curr != NULL; curr = curr->next) {
        formatOperand(curr->arg1, a1, sizeof(a1));
        formatOperand(curr->arg2, a2, sizeof(a2));
        formatOperand(curr->result, res, sizeof(res));

        switch (curr->op) {
            case TAC_ADD:        fprintf(out, "    %s = %s + %s\n", res, a1, a2); break;
            case TAC_SUB:        fprintf(out, "    %s = %s - %s\n", res, a1, a2); break;
            case TAC_MUL:        fprintf(out, "    %s = %s * %s\n", res, a1, a2); break;
            case TAC_DIV:        fprintf(out, "    %s = %s / %s\n", res, a1, a2); break;
            case TAC_MOD:        fprintf(out, "    %s = %s %% %s\n", res, a1, a2); break;
            case TAC_NEG:        fprintf(out, "    %s = -%s\n", res, a1); break;
            case TAC_AND:        fprintf(out, "    %s = %s && %s\n", res, a1, a2); break;
            case TAC_OR:         fprintf(out, "    %s = %s || %s\n", res, a1, a2); break;
            case TAC_NOT:        fprintf(out, "    %s = !%s\n", res, a1); break;
            case TAC_LT:         fprintf(out, "    %s = %s < %s\n", res, a1, a2); break;
            case TAC_GT:         fprintf(out, "    %s = %s > %s\n", res, a1, a2); break;
            case TAC_EQ:         fprintf(out, "    %s = %s == %s\n", res, a1, a2); break;
            case TAC_ASSIGN:     fprintf(out, "    %s = %s\n", res, a1); break;
            case TAC_LABEL:      fprintf(out, "%s:\n", res); break;
            case TAC_GOTO:       fprintf(out, "    GOTO %s\n", res); break;
            case TAC_IF_FALSE:   fprintf(out, "    IF_FALSE %s GOTO %s\n", a1, res); break;
            case TAC_FUNC_BEGIN: fprintf(out, "\nFUNC_BEGIN %s:\n", a1); break;
            case TAC_FUNC_END:   fprintf(out, "FUNC_END %s\n", a1); break;
            case TAC_PARAM:      fprintf(out, "    PARAM %s\n", a1); break;
            case TAC_CALL:
                if (curr->result != NULL)
                    fprintf(out, "    %s = CALL %s\n", res, a1);
                else
                    fprintf(out, "    CALL %s\n", a1);
                break;
            case TAC_RETURN:
                if (curr->arg1 != NULL)
                    fprintf(out, "    RETURN %s\n", a1);
                else
                    fprintf(out, "    RETURN\n");
                break;
        }
    }
}