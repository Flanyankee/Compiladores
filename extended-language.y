%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syntax-tree.h"
#include "symbols-tab.h"
#include "printers.h"

extern int yylex();
void yyerror(const char *s);

ASTNode* tree;
SymbolTab* tab;

int evaluate(ASTNode* node);
%}

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
	
        if (findSymbol(s, tab) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s, tab);
	$$ = makeNode(s, $6, NULL);
	tree = $$;
	
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
    } | T_RETURN E T_PUNTO_COMA { 
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
        Symbol* s = makeSymbol($2, "VAR", $1);

        if (findSymbol(s, tab) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s, tab);

        $$ = makeNode(makeSymbol("DECL", "", ""), NULL, $4);
        
    }
    | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple { 
        Symbol* s = makeSymbol($2, "VAR", $1);

        if (findSymbol(s, tab) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s, tab);

        Symbol* asig = makeSymbol("=", "OP", $1);
        ASTNode* assignNode = makeNode(asig, makeLeaf(s), $4);
        
        $$ = makeNode(makeSymbol("DECL", "", ""), assignNode, $6);
    }
    | /* Produccion vacia */ { $$ = NULL; }
    ;

E_doble 
    : T_ID OP_ASIGN E T_PUNTO_COMA {
        Symbol sAux;
        sAux.id = $1;
        Symbol* s = findSymbol(&sAux, tab); // Buscamos con el temporal
        
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

E   : E OP_SUMA E           { $$ = makeNode(makeSymbol("+", "", "int"), $1, $3); }
    | E OP_MULT E           { $$ = makeNode(makeSymbol("*", "", "int"), $1, $3); }
    | T_PAR_IZQ E T_PAR_DER { $$ = $2; }
    | E OP_AND E            { $$ = makeNode(makeSymbol("&&", "", "bool"), $1, $3); }
    | E OP_OR E             { $$ = makeNode(makeSymbol("||", "", "bool"), $1, $3); }
    | T_NUM                 { $$ = makeLeaf(makeSymbol($1, "CONST", "int")); }
    | T_BOOL                { $$ = makeLeaf(makeSymbol($1, "CONST", "bool")); }
    | T_ID                  { 
        Symbol sAux;
        sAux.id = $1;
        Symbol* s = findSymbol(&sAux, tab); 
        
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

    evaluate(tree);
    printDOT(tree);
    printSymbolTab(tab);


    FILE* assemblyFile = fopen("pseudo-assembly.txt", "w");
	if (assemblyFile == NULL) {
	    fprintf(stderr, "Error: No se pudo crear el archivo pseudo-assembly.txt\n");
		exit(1);
	}
    generateAssembly(tree, assemblyFile);
    fclose(assemblyFile);
    return 0;
}

int evaluate(ASTNode* node) {
    if (!node) return 0;
    
    Symbol* symbol = node->symbolData;

    // Nodos estructurales (recorremos a los hijos)
    if (strcmp(symbol->id, "Body") == 0 || 
        strcmp(symbol->id, "Statement") == 0 ||
        strcmp(symbol->symbolType, "FUNC") == 0 ||
        strcmp(symbol->symbolType, "DECL") == 0) {
        evaluate(node->left);
        evaluate(node->right);
        return 0;
    }
    
    // Nodos Constantes (retornan su valor)
    if (strcmp(symbol->symbolType, "CONST") == 0) {
        if (strcmp(symbol->type, "int") == 0)
            return atoi(symbol->id);
        else if (strcmp(symbol->type, "bool") == 0)
            return (strcmp(symbol->id, "true") == 0) ? 1 : 0;
    }
    
    // Nodos de Variable (acceden al valor guardado en el símbolo de la tabla)
    if (strcmp(symbol->symbolType, "VAR") == 0) {
        if (!symbol->hasValue) {
            fprintf(stderr, "\nError de Ejecucion: Variable '%s' sin inicializar.\n", symbol->id);
            exit(1);
        }
        return symbol->value.intVal;
    }
    
    // Nodos de Asignación (=) -> Actualizan la variable apuntada
    if (strcmp(symbol->id, "=") == 0) {
        int val = evaluate(node->right); 
        (node->left)->symbolData->value.intVal = val;
        (node->left)->symbolData->hasValue = 1;
        return val;
    }

    // Nodo de Retorno (RETURN)
    if (strcmp(symbol->symbolType, "RET") == 0) {
        int retVal = evaluate(node->right);
        return retVal;
    }

    
    // Nodos de Operaciones Matemáticas / Lógicas
    if (node->left != NULL && node->right != NULL) {
    	if (strcmp(symbol->type, node->left->symbolData->type) != 0
    		|| strcmp(symbol->type, node->right->symbolData->type) != 0) {
			fprintf(stderr, "\nError de tipos\n", symbol->id);
			exit(1);
	} else {
    		if (strcmp(symbol->id, "+") == 0)
    		    return evaluate(node->left) + evaluate(node->right);
    		if (strcmp(symbol->id, "*") == 0)
    		    return evaluate(node->left) * evaluate(node->right);
    		if (strcmp(symbol->id, "&&") == 0)
    		    return evaluate(node->left) && evaluate(node->right);
    		if (strcmp(symbol->id, "||") == 0)
    		    return evaluate(node->left) || evaluate(node->right);
    	}
    }
        
    return 0;
}  

