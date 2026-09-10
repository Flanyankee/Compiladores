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
ASTNode* tree;

Symbol* makeSymbol(char* id, char* symType, char* dType);
ASTNode* makeNode(Symbol* symbol, ASTNode* left, ASTNode* right);
ASTNode* makeLeaf(Symbol* symbol);

SymbolTab* tab;

SymbolTab* initializeSymbolTab();
void addSymbolToTab(Symbol* symbol);
Symbol* findSymbol(Symbol* symbol);

int evaluate(ASTNode* node);

void generateAssembly(ASTNode* node);

/* Funci�n para imprimir en formato DOT */
void printDOTEdges(ASTNode* node, FILE* file);
void printDOT(ASTNode* root);
void printSymbolTab(SymbolTab* tab);
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
	
        if (findSymbol(s) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s);
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

        if (findSymbol(s) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s);

        $$ = makeNode(makeSymbol("DECL", "", ""), NULL, $4);
        
    }
    | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple { 
        Symbol* s = makeSymbol($2, "VAR", $1);

        if (findSymbol(s) != NULL) yyerror("Error: Variable ya declarada");
        addSymbolToTab(s);

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

    evaluate(tree);
    generateAssembly(tree);
    printDOT(tree);
    printSymbolTab(tab);
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

void printDOTEdges(ASTNode* node, FILE* file) {
    if (!node) return;
    
    fprintf(file, "  node%d [shape=box, label=\"ID: %s\\nSymType: %s\\nDataType: %s", 
           node->internalId, 
           node->symbolData->id, 
           node->symbolData->symbolType, 
           node->symbolData->type);
    
    if (node->symbolData->hasValue) {
        if (strcmp(node->symbolData->type, "int") == 0) {
            fprintf(file, "\\nValue: %d", node->symbolData->value.intVal); 
        } else if (strcmp(node->symbolData->type, "bool") == 0) {
            fprintf(file, "\\nValue: %s", node->symbolData->value.boolVal ? "true" : "false");
        }
    }
    fprintf(file, "\"];\n");    

    if (node->left) {
        fprintf(file, "  node%d -> node%d;\n", node->internalId, node->left->internalId);
        printDOTEdges(node->left, file);
    }
    if (node->right) {
        fprintf(file, "  node%d -> node%d;\n", node->internalId, node->right->internalId);
        printDOTEdges(node->right, file);
    }
}

void printDOT(ASTNode* root) {
    FILE* outputFile = fopen("syntax-tree.dot", "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo arbol_sintactico.txt\n");
        return;
    }

    fprintf(outputFile, "digraph AST {\n");
    fprintf(outputFile, "  node [fontname=\"Arial\"];\n");
    printDOTEdges(root, outputFile);
    fprintf(outputFile, "}\n");
    fclose(outputFile);
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

void generateAssembly(ASTNode* node) {
    if (!node) return;
    
    if (strcmp(node->symbolData->id, "Body") == 0 || 
        strcmp(node->symbolData->id, "Statement") == 0) {
        generateAssembly(node->left);
        generateAssembly(node->right);
        return;
    }

    if (strcmp(node->symbolData->symbolType, "FUNC") == 0) {
        printf(".globl %s\n", node->symbolData->id);
        printf("%s:\n", node->symbolData->id);
        generateAssembly(node->left);
        return;
    }

    if (strcmp(node->symbolData->symbolType, "CONST") == 0) {
        int val = 0;
        if (strcmp(node->symbolData->type, "int") == 0)
            val = atoi(node->symbolData->id);
        else if (strcmp(node->symbolData->type, "bool") == 0)
            val = (strcmp(node->symbolData->id, "true") == 0) ? 1 : 0;  
        printf("  movq $%d, %%rax\n", val);
        return;
    }

    if (strcmp(node->symbolData->symbolType, "VAR") == 0) {
        if (node->left == NULL && node->right == NULL) { // Solo si es hoja
            printf("  movq (%%rbp), %%rax\n");
        }
        return;
    }
    
    if (strcmp(node->symbolData->id, "=") == 0) {
        generateAssembly(node->right); 
        
        ASTNode* varNode = node->left; // El hijo izquierdo es la variable
        // Movemos el resultado de %rax a la ubicación de memoria de la variable
        printf("  movq %%rax, (%%rbp)\n");
        return;
    }
    
    if (strcmp(node->symbolData->id, "+") == 0 ||
        strcmp(node->symbolData->id, "-") == 0 ||
        strcmp(node->symbolData->id, "*") == 0 ||
        strcmp(node->symbolData->id, "&&") == 0 ||
        strcmp(node->symbolData->id, "||") == 0) {
        
        generateAssembly(node->left);    
        printf("  pushq %%rax\n");      
        
        generateAssembly(node->right);   
        printf("  movq %%rax, %%r10\n"); //Movemos la derecha al registro temporal %r10
        printf("  popq %%rax\n");        //Recuperamos la izquierda en %rax
        
        if (strcmp(node->symbolData->id, "+") == 0) {
            printf("  addq %%r10, %%rax\n");
        } else if (strcmp(node->symbolData->id, "-") == 0) {
            printf("  subq %%r10, %%rax\n");
        } else if (strcmp(node->symbolData->id, "*") == 0) {
            printf("  imulq %%r10, %%rax\n"); 
        } else if (strcmp(node->symbolData->id, "&&") == 0) {
            printf("  andq %%r10, %%rax\n");
        } else if (strcmp(node->symbolData->id, "||") == 0) {
            printf("  orq %%r10, %%rax\n");
        }
        return;
    }
    
    if (strcmp(node->symbolData->symbolType, "RET") == 0) {
        if (node->right) {
            generateAssembly(node->right); 
        }
        printf("  leave\n");
        printf("  ret\n");
        return;
    }
}

void printSymbolTab(SymbolTab* tab) {
	FILE* outputFile = fopen("symbols-tab.txt", "w");
	if (outputFile == NULL) {
		fprintf(stderr, "Error: No se pudo crear el archivo arbol_sintactico.txt\n");
		return;
	}

	SymbolTabElem* aux = (tab->first)->next;
	while (aux->next != NULL) {
		Symbol* auxSymbol = aux->symbol;

		if (auxSymbol->hasValue == 1) {
			fprintf(outputFile, "SymType: %s, ID: %s, Type: %s, Value: %d", auxSymbol->symbolType, auxSymbol->id, auxSymbol->type, auxSymbol->value.intVal);
		} else {
			fprintf(outputFile, "SymType: %s, ID: %s, Type: %s", auxSymbol->symbolType, auxSymbol->id, auxSymbol->type);
		}
		fprintf(outputFile, "\t=>\t");

		aux = aux->next;
	}
	fprintf(outputFile, "\n");

	fclose(outputFile);
}
