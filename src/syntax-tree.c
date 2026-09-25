#include <stdlib.h>
#include <string.h>
#include "syntax-tree.h"

int nodeCount = 0;

/*
 * Inicio de las definiciones de funciones.
 */
ASTNode* makeNode(Symbol* symbol, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->internalId = nodeCount++;
    node->symbolData = symbol;
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* makeLeaf(Symbol* symbol) {
    return makeNode(symbol, NULL, NULL);
}

Symbol* makeSymbol(char* id, char* symType, char* dType) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->symbolType = symType;
    symbol->id = strdup(id);
    symbol->type = dType;
    symbol->hasValue = 0;
    return symbol;
}
