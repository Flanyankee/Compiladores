#ifndef SEMANTIC_ANALIZER
#define SEMANTIC_ANALIZER

#include "syntax-tree.h"
#include "symbols-tab.h"
#include "constants.h"

// Función principal para iniciar el análisis semántico
void analyze_ast(ASTNode *node, SymbolTab *tab);

// Función auxiliar para evaluar expresiones y retornar su tipo
enum types analyze_expression(ASTNode *expr, SymbolTab *tab);

#endif // SEMANTIC_ANALIZER