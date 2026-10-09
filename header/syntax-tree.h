#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H
#include "constants.h"

typedef struct Symbol {
	enum symbolTypes symbolType; // si es variable, funcion, etc.
	char *id;
	enum types type; // si es int, bool o void.
	union {
		int intValue;
		int boolValue;
		float floatValue;
	} value;
	int hasValue;
} Symbol;

typedef struct ASTNode {
	Symbol *symbolData;
	struct ASTNode *left;
	struct ASTNode *right;
} ASTNode;

/** Variable global para la definicion (en el parser) de varias variables del
 * mismo tipo.
 * i.e: int x1, x2, x3, ...
 */
extern char *declType;

extern ASTNode *syntaxTree;

ASTNode *makeNode(Symbol *symbol, ASTNode *left, ASTNode *right);
ASTNode *makeLeaf(Symbol *symbol);
Symbol *makeSymbol(char *id, enum symbolTypes symType);
#endif
