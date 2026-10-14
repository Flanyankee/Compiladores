#ifndef SEMANTIC_ANALIZER
#define SEMANTIC_ANALIZER

#include "constants.h"
#include "symbols-tab.h"
#include "syntax-tree.h"
#include <stdio.h>

extern FILE *semanticOutputFile;

// Función principal para iniciar el análisis semántico
int analyzeAst(ASTNode *node, SymbolTab *tab);

// Función auxiliar para evaluar expresiones y retornar su tipo
enum types analyzeExpression(ASTNode *expr, SymbolTab *tab);

static enum types evaluateFunctionInvoke(ASTNode *expr, SymbolTab *tab, int isExpression);

#endif // SEMANTIC_ANALIZER
