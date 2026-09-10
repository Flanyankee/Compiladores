#include "printers.h"
#include <stdlib.h>
#include <string.h>

void printDOTEdges(ASTNode* node, FILE* outputFile) {
    if (!node) return;
    
    fprintf(outputFile, "  node%d [shape=box, label=\"ID: %s\\nSymType: %s\\nDataType: %s", 
           node->internalId, 
           node->symbolData->id, 
           node->symbolData->symbolType, 
           node->symbolData->type);
    
    if (node->symbolData->hasValue) {
        if (strcmp(node->symbolData->type, "int") == 0) {
            fprintf(outputFile, "\\nValue: %d", node->symbolData->value.intVal); 
        } else if (strcmp(node->symbolData->type, "bool") == 0) {
            fprintf(outputFile, "\\nValue: %s", node->symbolData->value.boolVal ? "true" : "false");
        }
    }
    fprintf(outputFile, "\"];\n");    

    if (node->left) {
        fprintf(outputFile, "  node%d -> node%d;\n", node->internalId, node->left->internalId);
        printDOTEdges(node->left, outputFile);
    }
    if (node->right) {
        fprintf(outputFile, "  node%d -> node%d;\n", node->internalId, node->right->internalId);
        printDOTEdges(node->right, outputFile);
    }
}

void printDOT(ASTNode* root) {
    FILE* outputFile = fopen("syntax-tree.dot", "w");
    if (outputFile == NULL) {
        fprintf(stderr, "Error: No se pudo crear el archivo syntax-tree.dot\n");
        return;
    }

    fprintf(outputFile, "digraph AST {\n");
    fprintf(outputFile, "  node [fontname=\"Arial\"];\n");
    printDOTEdges(root, outputFile);
    fprintf(outputFile, "}\n");
    fclose(outputFile);
}

void generateAssembly(ASTNode* node, FILE* outputFile) {
    if (!node) return;
    

    Symbol* symbol = node->symbolData;

    if (strcmp(symbol->id, "Body") == 0 || 
        strcmp(symbol->id, "Statement") == 0) {
        generateAssembly(node->left, outputFile);
        generateAssembly(node->right, outputFile);
        return;
    }

    if (strcmp(symbol->symbolType, "FUNC") == 0) {
        fprintf(outputFile, ".globl %s\n", symbol->id);
        fprintf(outputFile, "%s:\n", symbol->id);
        generateAssembly(node->left, outputFile);
        return;
    }

    if (strcmp(symbol->symbolType, "CONST") == 0) {
        int val = 0;
        if (strcmp(symbol->type, "int") == 0)
            val = atoi(symbol->id);
        else if (strcmp(symbol->type, "bool") == 0)
            val = (strcmp(symbol->id, "true") == 0) ? 1 : 0;  
        fprintf(outputFile, "  movq $%d, %%rax\n", val);
        return;
    }

    if (strcmp(symbol->symbolType, "VAR") == 0) {
        if (node->left == NULL && node->right == NULL) { // Solo si es hoja
            fprintf(outputFile, "  movq (%%rbp), %%rax\n");
        }
        return;
    }
    
    if (strcmp(symbol->id, "=") == 0) {
        generateAssembly(node->right, outputFile); 
        
        ASTNode* varNode = node->left; // El hijo izquierdo es la variable
        // Movemos el resultado de %rax a la ubicación de memoria de la variable
        fprintf(outputFile, "  movq %%rax, (%%rbp)\n");
        return;
    }
    
    if (strcmp(symbol->id, "+") == 0 ||
        strcmp(symbol->id, "-") == 0 ||
        strcmp(symbol->id, "*") == 0 ||
        strcmp(symbol->id, "&&") == 0 ||
        strcmp(symbol->id, "||") == 0) {
        
        generateAssembly(node->left, outputFile);    
        fprintf(outputFile, "  pushq %%rax\n");      
        
        generateAssembly(node->right, outputFile);   
        fprintf(outputFile, "  movq %%rax, %%r10\n"); //Movemos la derecha al registro temporal %r10
        fprintf(outputFile, "  popq %%rax\n");        //Recuperamos la izquierda en %rax
        
        if (strcmp(symbol->id, "+") == 0) {
            fprintf(outputFile, "  addq %%r10, %%rax\n");
        } else if (strcmp(symbol->id, "-") == 0) {
            fprintf(outputFile, "  subq %%r10, %%rax\n");
        } else if (strcmp(symbol->id, "*") == 0) {
            fprintf(outputFile, "  imulq %%r10, %%rax\n"); 
        } else if (strcmp(symbol->id, "&&") == 0) {
            fprintf(outputFile, "  andq %%r10, %%rax\n");
        } else if (strcmp(symbol->id, "||") == 0) {
            fprintf(outputFile, "  orq %%r10, %%rax\n");
        }
        return;
    }
    
    if (strcmp(symbol->symbolType, "RET") == 0) {
        if (node->right) {
            generateAssembly(node->right, outputFile); 
        }
        fprintf(outputFile, "  leave\n");
        fprintf(outputFile, "  ret\n");
        return;
    }
}

void printSymbolTab(SymbolTab* tab) {
	FILE* outputFile = fopen("symbols-tab.txt", "w");
	if (outputFile == NULL) {
		fprintf(stderr, "Error: No se pudo crear el archivo symbols-tab.txt\n");
		return;
	}

	SymbolTabElem* aux = (tab->first)->next;
	while (aux->next != NULL) {
		Symbol* auxSymbol = aux->symbol;

		fprintf(outputFile, "SymType: %s, ID: %s, Type: %s", auxSymbol->symbolType, auxSymbol->id, auxSymbol->type);
		if (auxSymbol->hasValue == 1) {
			fprintf(outputFile, ", Value: %d",  auxSymbol->value.intVal);
		}
		fprintf(outputFile, "\t=>\t");

		aux = aux->next;
	}
	fprintf(outputFile, "\n");

	fclose(outputFile);
}
