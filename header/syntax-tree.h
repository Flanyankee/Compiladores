#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
#include "constants.h"

typedef struct Symbol {
	enum symbolTypes symbolType; // si es variable, funcion, etc.
	char *id;
	enum types type; // si es int, bool o void.
	int value;
	int hasValue;
} Symbol;

typedef struct ASTNode {
	Symbol *symbolData;
	struct ASTNode *left;
	struct ASTNode *right;
} ASTNode;

ASTNode *makeNode(Symbol *symbol, ASTNode *left, ASTNode *right);
ASTNode *makeLeaf(Symbol *symbol);
Symbol *makeSymbol(char *id, enum symbolTypes symType, enum types valueType);
#endif
