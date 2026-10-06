#include "semantic-analizer.h"
#include "constants.h"
#include <stdio.h>
#include <string.h>

void analyze_ast(ASTNode *node, SymbolTab *tab) {
    Symbol* symbol = node->symbolData;

    if (node == NULL) return;

    if (symbol->symbolType == program) {
        analyze_ast(node->left, tab);
        analyze_ast(node->right, tab);
        return;
    }

    switch (symbol->symbolType) {

    
        case(varDeclaration):
            Symbol *existing = findSymbol(symbol, tab);
                if (existing != NULL && existing->symbolType == symbol->symbolType) {
                    printf("Error Semántico: La variable '%s' ya fue declarada.\n", symbol->id);
                } else {
                    addSymbolToTab(symbol, tab);
                }
            break;
    }

}