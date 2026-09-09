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

typedef struct SymbolTabElem {
	Symbol* symbol;
	struct SymbolTabElem* next;
	struct SymbolTabElem* back;
} SymbolTabElem;

typedef struct SymbolTab {
	struct SymbolTabElem* first;
	struct SymbolTabElem* last;
} SymbolTab;

int nodeCount = 0;

Symbol* makeSymbol(char* id, char* symType, char* dType);
ASTNode* makeNode(Symbol* symbol, ASTNode* left, ASTNode* right);
ASTNode* makeLeaf(Symbol* symbol);

SymbolTab* tab;

SymbolTab* initializeSymbolTab();
void addSymbolToTab(Symbol* symbol);
Symbol* findSymbol(Symbol* symbol);

int evaluate(ASTNode* node);

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
	$$ = makeNode(s, $6, NULL);
        printDOT($$);

        evaluate($$);
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
    	Symbol* s = makeSymbol($1, "RET", "void");
        $$ = makeLeaf(s); 
    }
    | T_RETURN E T_PUNTO_COMA { 
    /* 
    	Hago lo mismo pero asignandole el tipo al return
	de la expresion que devuelve.
    */
	    Symbol* s = makeSymbol($1, "RET", $2->symbolData->type);
        $$ = makeNode(s, NULL, $2); 
    }
    ;

E_triple 
    : T_TYPE T_ID T_PUNTO_COMA E_triple { 
        Symbol sAux;
        sAux.id = $2;
        if (findSymbol(&sAux) != NULL) yyerror("Error: Variable ya declarada");

        Symbol* s = makeSymbol($2, "VAR", $1);
        addSymbolToTab(s);
        $$ = makeNode(s, NULL, $4);
        
    }
    | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple { 
	Symbol sAux;
        sAux.id = $2;
        if (findSymbol(&sAux) != NULL) yyerror("Error: Variable ya declarada");

        Symbol* s = makeSymbol($2, "VAR", $1);
        addSymbolToTab(s); 
        
        Symbol* asig = makeSymbol("=", "OP", $1);
        ASTNode* assignNode = makeNode(asig, makeLeaf(s), $4);
        
        $$ = makeNode(s, assignNode, $6);
    }
    | /* Produccin vaca */ { $$ = NULL; }
    ;

E_doble 
    : T_ID OP_ASIGN E T_PUNTO_COMA {
        Symbol sAux;
        sAux.id = $1;
        Symbol* s = findSymbol(&sAux); // Buscamos con el temporal
        
        if (s == NULL) yyerror("Error: Variable no declarada");
        
        Symbol* asig = makeSymbol("=", "OP", s->type);
        $$ = makeNode(asig, makeLeaf(s), $3);
    }
    | R {$$ = $1;}
    ;

E_prima 
    : E_doble E_prima { $$ = makeNode(makeSymbol("Statement", "", ""), $1, $2); }
    | /* Producci�n vac�a */ { $$ = NULL; }
    ;

E   : E OP_SUMA E           { $$ = makeNode(makeSymbol("+", "", ""), $1, $3); }
    | E OP_MULT E           { $$ = makeNode(makeSymbol("*", "", ""), $1, $3); }
    | T_PAR_IZQ E T_PAR_DER { $$ = $2; }
    | E OP_AND E            { $$ = makeNode(makeSymbol("&&", "", ""), $1, $3); }
    | E OP_OR E             { $$ = makeNode(makeSymbol("||", "", ""), $1, $3); }
    | T_NUM                 { $$ = makeLeaf(makeSymbol($1, "CONST", "int")); }
    | T_BOOL                { $$ = makeLeaf(makeSymbol($1, "CONST", "bool")); }
    | T_ID                  { 
        Symbol sAux;
        sAux.id = $1;
        Symbol* s = findSymbol(&sAux); 
        
        if (s == NULL) yyerror("Error: Variable no declarada");
        $$ = makeLeaf(s);
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "\n error de sintaxis: %s\n", s);
    exit(1);
}

int main() {
    tab = initializeSymbolTab();
    yyparse();
    return 0;
}

Symbol* makeSymbol(char* id, char* symType, char* dType) {
    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    symbol->symbolType = symType;
    symbol->id = strdup(id);
    symbol->type = dType;
    symbol->hasValue = 0;
    return symbol;
}

SymbolTab* initializeSymbolTab() {
	SymbolTab* tab = (SymbolTab*)malloc(sizeof(SymbolTab));
	tab->first = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	tab->last = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	(tab->first)->next = tab->last;
	(tab->first)->back = NULL;
	(tab->last)->back = tab->first;
	(tab->last)->next = NULL;
	return tab;
}
void addSymbolToTab(Symbol* symbol) {
	SymbolTabElem* elem = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	elem->symbol = symbol;
	elem->back = (tab->last)->back;
	elem->next = tab->last;
	(elem->back)->next = elem;
	(tab->last)->back = elem;
}
Symbol* findSymbol(Symbol* symbol) {
	SymbolTabElem* aux = tab->first->next;
	while (aux->next != NULL) {
		if (strcmp(aux->symbol->id, symbol->id) == 0) {
			return aux->symbol;
		}
		aux = aux->next;
	}
	return NULL;
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

int evaluate(ASTNode* node) {
    if (!node) return 0;
    
    // Nodos estructurales (recorremos a los hijos)
    if (strcmp(node->symbolData->id, "Body") == 0 || 
        strcmp(node->symbolData->id, "Statement") == 0 ||
        strcmp(node->symbolData->symbolType, "FUNC") == 0 ||
        strcmp(node->symbolData->symbolType, "VAR") == 0) {
        evaluate(node->left);
        evaluate(node->right);
        return 0;
    }
    
    // Nodos Constantes (retornan su valor)
    if (strcmp(node->symbolData->symbolType, "CONST") == 0) {
        if (strcmp(node->symbolData->type, "int") == 0)
            return atoi(node->symbolData->id);
        else if (strcmp(node->symbolData->type, "bool") == 0)
            return (strcmp(node->symbolData->id, "true") == 0) ? 1 : 0;
    }
    
    // Nodos de Variable (acceden al valor guardado en el símbolo de la tabla)
    if (strcmp(node->symbolData->symbolType, "VAR") == 0) {
        if (!node->symbolData->hasValue) {
            fprintf(stderr, "\nError de Ejecucion: Variable '%s' sin inicializar.\n", node->symbolData->id);
            exit(1);
        }
        return node->symbolData->value.intVal;
    }
    
    // Nodos de Asignación (=) -> Actualizan la variable apuntada
    if (strcmp(node->symbolData->id, "=") == 0) {
        int val = evaluate(node->right); 
        node->left->symbolData->value.intVal = val;
        node->left->symbolData->hasValue = 1;
        return val;
    }
    
    // Nodos de Operaciones Matemáticas / Lógicas
    if (strcmp(node->symbolData->id, "+") == 0)
        return evaluate(node->left) + evaluate(node->right);
    if (strcmp(node->symbolData->id, "-") == 0)
        return evaluate(node->left) - evaluate(node->right);
    if (strcmp(node->symbolData->id, "*") == 0)
        return evaluate(node->left) * evaluate(node->right);
    if (strcmp(node->symbolData->id, "&&") == 0)
        return evaluate(node->left) && evaluate(node->right);
    if (strcmp(node->symbolData->id, "||") == 0)
        return evaluate(node->left) || evaluate(node->right);
        
    // Nodo de Retorno (RETURN)
    if (strcmp(node->symbolData->symbolType, "RET") == 0) {
        int retVal = evaluate(node->right);
        printf(">>> Programa retorna el valor: %d\n", retVal);
        return retVal;
    }

    return 0;
}    