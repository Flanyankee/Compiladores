%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
void yyerror(const char *s);

typedef struct Symbol {
    char* symbolType; // si es variable, funcion, etc.
    char* id;
    char* type;     // si es int, bool o void.

    union {
    	int intVal;
	int boolVal;
    } value;

    int hasValue;
} Symbol;

typedef struct ASTNode {
    int internalId;
    Symbol* symbolData;
    struct ASTNode* left;
    struct ASTNode* right;
} ASTNode;

int nodeCount = 0;

Symbol* makeSymbol(char* id, char* symType, char* dType);
ASTNode* makeNode(Symbol* symbol, ASTNode* left, ASTNode* right);
ASTNode* makeLeaf(Symbol* symbol);

/* Funci�n para imprimir en formato DOT */
void printDOTEdges(ASTNode* node);
void printDOT(ASTNode* root);
%}

/* Tipos sem�nticos */
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
	Symbol* s = makeSymbol($2, "FUNC", $1);
        /* ASTNode* root = makeNode(s, makeLeaf($1), makeLeaf($2));
        $$ = makeNode("Program", root, $6); */
	$$ = makeNode(s, $6, NULL);
        printDOT($$);
    }
    ;

P   : E_triple E_prima { 
        $$ = makeNode(makeSymbol("Body", "", ""), $1, $2); 
    }
    ;

R   : T_RETURN T_PUNTO_COMA { 
    /* Lo hago para que el nodo tenga el simbolo return y nada mas.
    	Porque antes tenias el nodo con label "Return" y como hoja
	tenias el nodo correspondiente al return.
    */
    	Symbol* s = makeSymbol($1, "", "");
        $$ = makeLeaf(s); 
    }
    | T_RETURN E T_PUNTO_COMA { 
    /* 
    	Hago lo mismo pero asignandole el tipo al return
	de la expresion que devuelve.
    */
	Symbol* s = makeSymbol($1, "", $2->symbolData->type);
	s->value = $2->symbolData->value;
	s->hasValue = $2->symbolData->hasValue;
        $$ = makeNode(s, NULL, $2); 
    }
    ;

E_triple 
    : T_TYPE T_ID T_PUNTO_COMA E_triple { 
	Symbol* s = makeSymbol($2, "VAR", $1);
        $$ = makeNode(s, NULL, $4); 
    }
    | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple { 
	Symbol* s = makeSymbol($2, "VAR", $1);
	s->value = $4->symbolData->value;
	s->hasValue = 1;
	$$ = makeNode(s, NULL, $6);
    }
    | /* Producci�n vac�a */ { $$ = NULL; }
    ;

E_doble 
    : T_ID OP_ASIGN E T_PUNTO_COMA {	Symbol* s = makeSymbol($1, "VAR", $3->symbolData->type);
    					s->value = $3->symbolData->value;
    					s->hasValue = $3->symbolData->hasValue;
    					$$ = makeNode(s, NULL, $3); }
    | R {$$ = $1;}
    ;

E_prima 
    : E_doble E_prima { $$ = makeNode(makeSymbol("Statement", "", ""), $1, $2); }
    | /* Producci�n vac�a */ { $$ = NULL; }
    ;

E   : E OP_SUMA E           { Symbol* s = makeSymbol("+", "", $1->symbolData->type);
    				s->value.intVal = $1->symbolData->value.intVal + $3->symbolData->value.intVal;
				s->hasValue = 1;
    				$$ = makeNode(s, $1, $3); }
    | E OP_MULT E           { Symbol* s = makeSymbol("*", "", $1->symbolData->type);
    				s->value.intVal = $1->symbolData->value.intVal * $3->symbolData->value.intVal;
				s->hasValue = 1;
    				$$ = makeNode(s, $1, $3); }
    | T_PAR_IZQ E T_PAR_DER { $$ = $2; }
    | E OP_AND E            { Symbol* s = makeSymbol("&&", "", $1->symbolData->type);
    				s->value.boolVal = $1->symbolData->value.boolVal && $3->symbolData->value.boolVal;
				s->hasValue = 1;
    				$$ = makeNode(s, $1, $3); }
    | E OP_OR E             { Symbol* s = makeSymbol("||", "", $1->symbolData->type);
    				s->value.boolVal = $1->symbolData->value.boolVal || $3->symbolData->value.boolVal;
				s->hasValue = 1;
    				$$ = makeNode(s, $1, $3); }
    | T_NUM                 { $$ = makeLeaf(makeSymbol($1, "CONST", "int")); }
    | T_BOOL                { $$ = makeLeaf(makeSymbol($1, "CONST", "bool")); }
    | T_ID                  { $$ = makeLeaf(makeSymbol($1, "VAR", "")); }
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

Symbol* makeSymbol(char* id, char* symType, char* dType) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->symbolType = symType;
    symbol->id = strdup(id);
    symbol->type = dType;
    // symbol->value.intVal = 0;
    symbol->hasValue = 0;
    return symbol;
}

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

void printDOTEdges(ASTNode* node) {
    if (!node) return;
    
    printf("  node%d [shape=box, label=\"ID: %s\\nSymType: %s\\nDataType: %s", 
           node->internalId, 
           node->symbolData->id, 
           node->symbolData->symbolType, 
           node->symbolData->type);
    
    if (node->symbolData->hasValue) {
        if (strcmp(node->symbolData->type, "int") == 0) {
            printf("\\nValue: %d", node->symbolData->value.intVal); 
        } else if (strcmp(node->symbolData->type, "bool") == 0) {
            printf("\\nValue: %s", node->symbolData->value.boolVal ? "true" : "false");
        }
    }
    printf("\"];\n");    

    if (node->left) {
        printf("  node%d -> node%d;\n", node->internalId, node->left->internalId);
        printDOTEdges(node->left);
    }
    if (node->right) {
        printf("  node%d -> node%d;\n", node->internalId, node->right->internalId);
        printDOTEdges(node->right);
    }
}

void printDOT(ASTNode* root) {
    printf("digraph AST {\n");
    printf("  node [fontname=\"Arial\"];\n");
    printDOTEdges(root);
    printf("}\n");
}
