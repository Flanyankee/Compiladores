#ifndef TAC_H
#define TAC_H

#include "syntax-tree.h"
#include <stdio.h>

// Códigos de operación para el Código de Tres Direcciones
typedef enum {
    // Operaciones aritméticas (enteras)
    TAC_ADD,        // result = arg1 + arg2
    TAC_SUB,        // result = arg1 - arg2
    TAC_MUL,        // result = arg1 * arg2
    TAC_DIV,        // result = arg1 / arg2
    TAC_MOD,        // result = arg1 % arg2
    TAC_NEG,        // result = -arg1 (unario)

    // Operaciones lógicas y relacionales
    TAC_AND,        // result = arg1 && arg2
    TAC_OR,         // result = arg1 || arg2
    TAC_NOT,        // result = !arg1
    TAC_LT,         // result = arg1 < arg2
    TAC_GT,         // result = arg1 > arg2
    TAC_EQ,         // result = arg1 == arg2

    // Asignación y carga
    TAC_ASSIGN,     // result = arg1 (incluye copia de variables y constantes)

    // Control de flujo (saltos y etiquetas)
    TAC_LABEL,      // result: (definición de etiqueta Lx)
    TAC_GOTO,       // GOTO result (salto incondicional)
    TAC_IF_FALSE,   // IF_FALSE arg1 GOTO result (salto condicional)

    // Funciones y métodos
    TAC_FUNC_BEGIN, // Inicio de función
    TAC_FUNC_END,   // Fin de función
    TAC_PARAM,      // PARAM arg1 (pasar parámetro actual)
    TAC_CALL,       // result = CALL arg1 (invocación a función)
    TAC_RETURN      // RETURN arg1 (o vacío si arg1 == NULL)
} TACOp;

// Instrucción de tres direcciones (Cuádrupla) en lista enlazada
typedef struct TACInstruction {
    TACOp op;
    Symbol *arg1;
    Symbol *arg2;
    Symbol *result;
    struct TACInstruction *next;
} TACInstruction;

// Funciones principales del generador
void initTACGenerator(void);
void generateTACFromAST(ASTNode *root);
Symbol *generateExprTAC(ASTNode *expr);
void printTAC(FILE *out);
TACInstruction *getTACListHead(void);

#endif