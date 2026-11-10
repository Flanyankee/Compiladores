#ifndef OPTIMIZATIONS_H
#define OPTIMIZATIONS_H
#include "constants.h"
#include "syntax-tree.h"
#include <stddef.h>

void constantOptimizations();
void deadCodeOptimizations();
void memoryOptimizations();

ASTNode *constantOptimizationsHelper(ASTNode *n);
ASTNode *foldConstantOperation(ASTNode *node, Symbol *leftSym,
                               Symbol *rightSym);
Symbol *createLiteralSymbol(int isFloat, float resFloat, int resInt);
#endif // OPTIMIZATIONS_H
