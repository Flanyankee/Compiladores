#include "syntax-tree.h"
#include <stdlib.h>
#include <string.h>

ASTNode *makeNode(Symbol *symbol, ASTNode *left, ASTNode *right) {
	ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
	node->symbolData = symbol;
	node->left = left;
	node->right = right;
	return node;
}

ASTNode *makeLeaf(Symbol *symbol) { return makeNode(symbol, NULL, NULL); }

Symbol *makeSymbol(char *id, enum symbolTypes symType, enum types valueType) {
	Symbol *symbol = (Symbol *)malloc(sizeof(Symbol));
	symbol->symbolType = symType;
	symbol->id = strdup(id);
	symbol->type = valueType;
	symbol->value = 0;
	symbol->hasValue = 0;
	return symbol;
}
