#ifndef SEMANTIC_ANALIZER
#define SEMANTIC_ANALIZER

#include "constants.h"
#include "symbols-tab.h"
#include "syntax-tree.h"

// Función principal para iniciar el análisis semántico
int analyzeAst(ASTNode *node, SymbolTab *tab);

// Función auxiliar para evaluar expresiones y retornar su tipo
enum types analyzeExpression(ASTNode *expr, SymbolTab *tab);

#endif // SEMANTIC_ANALIZER
