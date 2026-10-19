#include "codinter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static CodinterInstruction *codinterHead = NULL;
static CodinterInstruction *codinterTail = NULL;
static int tempCounter = 0;
static int labelCounter = 0;

void initCodinterGenerator(void) {
    codinterHead = NULL;
    codinterTail = NULL;
    tempCounter = 0;
    labelCounter = 0;
}

CodinterInstruction *getCodinterListHead(void) {
    return codinterHead;
}

// Emite una instrucción al final de la lista lineal
static CodinterInstruction *emit(CodinterOp op, Symbol *arg1, Symbol *arg2, Symbol *result) {
    CodinterInstruction *inst = (CodinterInstruction *)malloc(sizeof(CodinterInstruction));
    inst->op = op;
    inst->arg1 = arg1;
    inst->arg2 = arg2;
    inst->result = result;
    inst->next = NULL;

    if (codinterHead == NULL) {
        codinterHead = inst;
        codinterTail = inst;
    } else {
        codinterTail->next = inst;
        codinterTail = inst;
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
Symbol *generateExprCodinter(ASTNode *expr) {
    if (expr == NULL || expr->symbolData == NULL) return NULL;

    Symbol *sym = expr->symbolData;

    switch (sym->symbolType) {
        case intLiteral:
        case boolLiteral:
        case floatLiteral: {
            // Opcional: puedes devolver directamente el literal o cargarlo en un temporal (Load_Const)
            Symbol *t = newTemp(sym->type);
            emit(Codinter_ASSIGN, sym, NULL, t);
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
                Symbol *op1 = generateExprCodinter(expr->left);
                Symbol *res = newTemp(op1->type);
                emit(Codinter_NEG, op1, NULL, res);
                return res;
            }

            Symbol *leftOp = generateExprCodinter(expr->left);
            Symbol *rightOp = generateExprCodinter(expr->right);

            enum types resType = tInt;
            if (sym->symbolType == lessOp || sym->symbolType == greaterOp ||
                sym->symbolType == equalsOp || sym->symbolType == andOp ||
                sym->symbolType == orOp) {
                resType = tBoolean;
            }

            Symbol *res = newTemp(resType);
            CodinterOp opCode;
            switch (sym->symbolType) {
                case addOp:       opCode = Codinter_ADD; break;
                case substractOp: opCode = Codinter_SUB; break;
                case multOp:      opCode = Codinter_MUL; break;
                case divOp:       opCode = Codinter_DIV; break;
                case modOp:       opCode = Codinter_MOD; break;
                case lessOp:      opCode = Codinter_LT;  break;
                case greaterOp:   opCode = Codinter_GT;  break;
                case equalsOp:    opCode = Codinter_EQ;  break;
                case andOp:       opCode = Codinter_AND; break;
                case orOp:        opCode = Codinter_OR;  break;
                default:          opCode = Codinter_ADD; break;
            }
            emit(opCode, leftOp, rightOp, res);
            return res;
        }

        case negationOp: {
            Symbol *op1 = generateExprCodinter(expr->left);
            Symbol *res = newTemp(tBoolean);
            emit(Codinter_NOT, op1, NULL, res);
            return res;
        }

        case functionInvoke: {
            // 1. Evaluar argumentos actuales y emitir PARAM
            ASTNode *currParam = expr->left;
            while (currParam != NULL && currParam->symbolData->symbolType == actualParameterList) {
                Symbol *argSym = generateExprCodinter(currParam->left);
                emit(Codinter_PARAM, argSym, NULL, NULL);
                currParam = currParam->right;
            }
            // 2. Emitir CALL
            if (sym->type != tVoid && sym->type != blank) {
                Symbol *res = newTemp(sym->type);
                emit(Codinter_CALL, sym, NULL, res);
                return res;
            } else {
                emit(Codinter_CALL, sym, NULL, NULL);
                return NULL;
            }
        }

        default:
            return sym;
    }
}

// Recorre sentencias, bloques y declaraciones del AST
void generateCodinterFromAST(ASTNode *node) {
    if (node == NULL || node->symbolData == NULL) return;

    Symbol *sym = node->symbolData;

    switch (sym->symbolType) {
        case program:
        case methodDeclarationList:
        case block:
        case statementList:
            generateCodinterFromAST(node->left);
            generateCodinterFromAST(node->right);
            break;

        case varDeclarationList:
        case varDeclaration:
            // Las declaraciones de variables ya están en la tabla de símbolos
            break;

        case function: {
            emit(Codinter_FUNC_BEGIN, sym, NULL, NULL);
            generateCodinterFromAST(node->right); // Cuerpo de la función (block)
            emit(Codinter_FUNC_END, sym, NULL, NULL);
            break;
        }

        case assignOp: {
            Symbol *rhs = generateExprCodinter(node->right);
            Symbol *lhs = node->left->symbolData;
            emit(Codinter_ASSIGN, rhs, NULL, lhs);
            break;
        }

        case ifNode: {
            Symbol *cond = generateExprCodinter(node->left);
            Symbol *labelFalse = newLabel();

            // Si la condición es falsa, salta a labelFalse
            emit(Codinter_IF_FALSE, cond, NULL, labelFalse);

            // Bloque THEN (hijo izquierdo del blankNode)
            ASTNode *ifBody = node->right;
            if (ifBody != NULL) {
                generateCodinterFromAST(ifBody->left);
            }

            // Verificar si existe rama ELSE (hijo derecho del blankNode)
            if (ifBody != NULL && ifBody->right != NULL) {
                Symbol *labelEnd = newLabel();
                emit(Codinter_GOTO, NULL, NULL, labelEnd);
                emit(Codinter_LABEL, NULL, NULL, labelFalse);
                // El bloque del elseNode está en ifBody->right->left
                generateCodinterFromAST(ifBody->right->left);
                emit(Codinter_LABEL, NULL, NULL, labelEnd);
            } else {
                emit(Codinter_LABEL, NULL, NULL, labelFalse);
            }
            break;
        }

        case whileNode: {
            Symbol *labelStart = newLabel();
            Symbol *labelEnd = newLabel();

            // Etiqueta de inicio antes de evaluar la condición
            emit(Codinter_LABEL, NULL, NULL, labelStart);
            Symbol *cond = generateExprCodinter(node->left);
            emit(Codinter_IF_FALSE, cond, NULL, labelEnd);

            // Cuerpo del while
            generateCodinterFromAST(node->right);
            emit(Codinter_GOTO, NULL, NULL, labelStart);

            // Etiqueta de salida del bucle
            emit(Codinter_LABEL, NULL, NULL, labelEnd);
            break;
        }

        case returnNode: {
            if (node->left != NULL) {
                Symbol *retVal = generateExprCodinter(node->left);
                emit(Codinter_RETURN, retVal, NULL, NULL);
            } else {
                emit(Codinter_RETURN, NULL, NULL, NULL);
            }
            break;
        }

        case functionInvoke: {
            generateExprCodinter(node);
            break;
        }

        default:
            generateCodinterFromAST(node->left);
            generateCodinterFromAST(node->right);
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

void printCodinter(FILE *out) {
    if (out == NULL) return;
    char a1[64], a2[64], res[64];

    for (CodinterInstruction *curr = codinterHead; curr != NULL; curr = curr->next) {
        formatOperand(curr->arg1, a1, sizeof(a1));
        formatOperand(curr->arg2, a2, sizeof(a2));
        formatOperand(curr->result, res, sizeof(res));

        switch (curr->op) {
            case Codinter_ADD:        fprintf(out, "    %s = %s + %s\n", res, a1, a2); break;
            case Codinter_SUB:        fprintf(out, "    %s = %s - %s\n", res, a1, a2); break;
            case Codinter_MUL:        fprintf(out, "    %s = %s * %s\n", res, a1, a2); break;
            case Codinter_DIV:        fprintf(out, "    %s = %s / %s\n", res, a1, a2); break;
            case Codinter_MOD:        fprintf(out, "    %s = %s %% %s\n", res, a1, a2); break;
            case Codinter_NEG:        fprintf(out, "    %s = -%s\n", res, a1); break;
            case Codinter_AND:        fprintf(out, "    %s = %s && %s\n", res, a1, a2); break;
            case Codinter_OR:         fprintf(out, "    %s = %s || %s\n", res, a1, a2); break;
            case Codinter_NOT:        fprintf(out, "    %s = !%s\n", res, a1); break;
            case Codinter_LT:         fprintf(out, "    %s = %s < %s\n", res, a1, a2); break;
            case Codinter_GT:         fprintf(out, "    %s = %s > %s\n", res, a1, a2); break;
            case Codinter_EQ:         fprintf(out, "    %s = %s == %s\n", res, a1, a2); break;
            case Codinter_ASSIGN:     fprintf(out, "    %s = %s\n", res, a1); break;
            case Codinter_LABEL:      fprintf(out, "%s:\n", res); break;
            case Codinter_GOTO:       fprintf(out, "    GOTO %s\n", res); break;
            case Codinter_IF_FALSE:   fprintf(out, "    IF_FALSE %s GOTO %s\n", a1, res); break;
            case Codinter_FUNC_BEGIN: fprintf(out, "\nFUNC_BEGIN %s:\n", a1); break;
            case Codinter_FUNC_END:   fprintf(out, "FUNC_END %s\n", a1); break;
            case Codinter_PARAM:      fprintf(out, "    PARAM %s\n", a1); break;
            case Codinter_CALL:
                if (curr->result != NULL)
                    fprintf(out, "    %s = CALL %s\n", res, a1);
                else
                    fprintf(out, "    CALL %s\n", a1);
                break;
            case Codinter_RETURN:
                if (curr->arg1 != NULL)
                    fprintf(out, "    RETURN %s\n", a1);
                else
                    fprintf(out, "    RETURN\n");
                break;
        }
    }
}