#ifndef PRINTERS_H
#define PRINTERS_H

#include "symbols-tab.h"
#include "syntax-tree.h"
#include <stdio.h>

void printDOTEdges(ASTNode *node, FILE *outputFile);
void printDOT(ASTNode *root);
void generateAssembly(ASTNode *node, FILE *outputFile);
void printSymbolTab(SymbolTab *tab);

#endif
