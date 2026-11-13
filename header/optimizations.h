#ifndef OPTIMIZATIONS_H
#define OPTIMIZATIONS_H

#include "constants.h"
#include "syntax-tree.h"
#include <stddef.h>

void constantOptimizations(enum optimizations optLevel);
void deadCodeOptimizations(enum optimizations optLevel);

ASTNode *constantOptimizationsHelper(ASTNode *n);
ASTNode *optimizeUnreachableCode(ASTNode *node);

ASTNode *foldConstantOperation(ASTNode *node, Symbol *leftSym,
                               Symbol *rightSym);
Symbol *createLiteralSymbol(int isFloat, int isBool, float resFloat,
                            int resInt);

void freeAST(ASTNode* node);
#endif // OPTIMIZATIONS_H
