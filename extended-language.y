%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
void yyerror(const char *s);

typedef struct Symbol {
    int symbolType; // si es variable, funcion, etc.
    char* id;
    char* type;     // si es int, bool o void.
    char* value;
} Symbol;

typedef struct ASTNode {
    int internal_id;
    struct Symbol* symbolData;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

int node_count = 0;

Symbol* makeSymbol(char* id) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->symbolType = 0;
    symbol->id = strdup(id);
    symbol->type = NULL;
    symbol->value = NULL;
    return symbol;
}

ASTNode* makeNode(char* id, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->internal_id = node_count++;
    node->symbolData = makeSymbol(id);
    node->left = left;
    node->right = right;
    return node;
}

ASTNode* makeLeaf(char* id) {
    return makeNode(id, NULL, NULL);
}

/* Función para imprimir en formato DOT */
void printDOT_Edges(ASTNode* node) {
    if (!node) return;
    
    printf("  node%d [label=\"%s\"];\n", node->internal_id, node->symbolData->id);
    
    if (node->left) {
        printf("  node%d -> node%d;\n", node->internal_id, node->left->internal_id);
        printDOT_Edges(node->left);
    }
    if (node->right) {
        printf("  node%d -> node%d;\n", node->internal_id, node->right->internal_id);
        printDOT_Edges(node->right);
    }
}

void printDOT(ASTNode* root) {
    printf("digraph AST {\n");
    printDOT_Edges(root);
    printf("}\n");
}
%}

/* Tipos semánticos */
%union {
    char* str;
    struct ASTNode* node;
}

%token <str> T_TYPE T_BOOL T_NUM T_ID T_RETURN 
%token OP_AND OP_OR OP_SUMA OP_MULT OP_RESTA OP_ASIGN 
%token T_PUNTO_COMA T_PAR_IZQ T_PAR_DER T_LLAVE_IZQ T_LLAVE_DER

%type <node> S P R E_triple E_doble E_prima E

%left OP_AND OP_OR
%left OP_SUMA OP_RESTA OP_MULT

%%

S   : T_TYPE T_ID T_PAR_IZQ T_PAR_DER T_LLAVE_IZQ P T_LLAVE_DER {
        ASTNode* root = makeNode($2, makeLeaf($1), makeLeaf($2));
        $$ = makeNode("Function", root, $6);
        printDOT($$);
    }
    ;

P   : E_triple E_prima { 
        $$ = makeNode("Body", $1, $2); 
    }
    ;

R   : T_RETURN T_PUNTO_COMA { 
        $$ = makeNode("Return", makeLeaf($1), NULL); 
    }
    | T_RETURN E T_PUNTO_COMA { 
        $$ = makeNode("Return", makeLeaf($1), $2); 
    }
    ;

E_triple 
    : T_TYPE T_ID T_PUNTO_COMA E_triple { 
        ASTNode* declaration = makeNode("Declaration", makeLeaf($1), makeLeaf($2));
        $$ = makeNode("Declaration List", declaration, $4); 
    }
    | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple { 
        ASTNode* declarationWAssign = makeNode("Declaration With Assign", makeLeaf($1), makeLeaf($2));
        ASTNode* expressionToAssign = makeNode("Expression to assign", declarationWAssign, $4);
        $$ = makeNode("Declaration List", expressionToAssign, $6);
    }
    | /* Producción vacía */ { $$ = NULL; }
    ;

E_doble 
    : T_ID OP_ASIGN E T_PUNTO_COMA { $$ = makeNode("Assign", makeLeaf($1), $3); }
    | R {$$ = $1;}
    ;

E_prima 
    : E_doble E_prima { $$ = makeNode("Statement", $1, $2); }
    | /* Producción vacía */ { $$ = NULL; }
    ;

E   : E OP_SUMA E           { $$ = makeNode("+", $1, $3); }
    | E OP_MULT E           { $$ = makeNode("*", $1, $3); }
    | T_PAR_IZQ E T_PAR_DER { $$ = $2; }
    | E OP_AND E            { $$ = makeNode("&&", $1, $3); }
    | E OP_OR E             { $$ = makeNode("||", $1, $3); }
    | T_NUM                 { $$ = makeLeaf($1); }
    | T_BOOL                { $$ = makeLeaf($1); }
    | T_ID                  { $$ = makeLeaf($1); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "\n error de sintaxis: %s\n", s);
    exit(1);
}

int main() {
    yyparse();
    return 0;
}
