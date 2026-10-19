#ifndef Codinter_H
#define Codinter_H

#include "syntax-tree.h"
#include <stdio.h>

// Códigos de operación para el Código de Tres Direcciones
typedef enum {
    // Operaciones aritméticas (enteras)
    Codinter_ADD,        // result = arg1 + arg2
    Codinter_SUB,        // result = arg1 - arg2
    Codinter_MUL,        // result = arg1 * arg2
    Codinter_DIV,        // result = arg1 / arg2
    Codinter_MOD,        // result = arg1 % arg2
    Codinter_NEG,        // result = -arg1 (unario)

    // Operaciones lógicas y relacionales
    Codinter_AND,        // result = arg1 && arg2
    Codinter_OR,         // result = arg1 || arg2
    Codinter_NOT,        // result = !arg1
    Codinter_LT,         // result = arg1 < arg2
    Codinter_GT,         // result = arg1 > arg2
    Codinter_EQ,         // result = arg1 == arg2

    // Asignación y carga
    Codinter_ASSIGN,     // result = arg1 (incluye copia de variables y constantes)

    // Control de flujo (saltos y etiquetas)
    Codinter_LABEL,      // result: (definición de etiqueta Lx)
    Codinter_GOTO,       // GOTO result (salto incondicional)
    Codinter_IF_FALSE,   // IF_FALSE arg1 GOTO result (salto condicional)

    // Funciones y métodos
    Codinter_FUNC_BEGIN, // Inicio de función
    Codinter_FUNC_END,   // Fin de función
    Codinter_PARAM,      // PARAM arg1 (pasar parámetro actual)
    Codinter_CALL,       // result = CALL arg1 (invocación a función)
    Codinter_RETURN      // RETURN arg1 (o vacío si arg1 == NULL)
} CodinterOp;

// Instrucción de tres direcciones (Cuádrupla) en lista enlazada
typedef struct CodinterInstruction {
    CodinterOp op;
    Symbol *arg1;
    Symbol *arg2;
    Symbol *result;
    struct CodinterInstruction *next;
} CodinterInstruction;

// Funciones principales del generador
void initCodinterGenerator(void);
void generateCodinterFromAST(ASTNode *root);
Symbol *generateExprCodinter(ASTNode *expr);
void printCodinter(FILE *out);
CodinterInstruction *getCodinterListHead(void);

#endif