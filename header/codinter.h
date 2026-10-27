#ifndef Codinter_H
#define Codinter_H

#include "syntax-tree.h"
#include <stdio.h>


typedef enum {
    Codinter_ADD,       
    Codinter_SUB,  
    Codinter_MUL,
    Codinter_DIV,
    Codinter_MOD, 
    Codinter_NEG,

    Codinter_AND,      
    Codinter_OR,         
    Codinter_NOT,
    Codinter_LT,
    Codinter_GT,  
    Codinter_EQ,  

    Codinter_ASSIGN,  

    Codinter_LABEL,    
    Codinter_GOTO,       
    Codinter_IF_FALSE, 

    Codinter_FUNC_BEGIN, 
    Codinter_FUNC_END,   
    Codinter_PARAM,     
    Codinter_CALL,       
    Codinter_RETURN     
} CodinterOp;

typedef struct CodinterInstruction {
    CodinterOp op;
    Symbol *arg1;
    Symbol *arg2;
    Symbol *result;
    struct CodinterInstruction *next;
} CodinterInstruction;

void initCodinterGenerator(void);
void generateCodinterFromAST(ASTNode *root);
Symbol *generateExprCodinter(ASTNode *expr);
void printCodinter(FILE *out);
CodinterInstruction *getCodinterListHead(void);

#endif