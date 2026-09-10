#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

typedef struct Symbol {
  char *symbolType; // si es variable, funcion, etc.
  char *id;
  char *type; // si es int, bool o void.

  union {
    int intVal;
    int boolVal;
  } value;

  int hasValue;
} Symbol;

typedef struct ASTNode {
  int internalId;
  Symbol *symbolData;
  struct ASTNode *left;
  struct ASTNode *right;
} ASTNode;

ASTNode *makeNode(Symbol *symbol, ASTNode *left, ASTNode *right);
ASTNode *makeLeaf(Symbol *symbol);
Symbol *makeSymbol(char *id, char *symType, char *dType);
#endif
