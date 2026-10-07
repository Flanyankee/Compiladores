#include "semantic-analizer.h"
#include "constants.h"
#include "symbols-tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static enum types current_method_return_type; 

void analyze_ast(ASTNode *node, SymbolTab *tab) {
    Symbol* symbol = node->symbolData;

    if (node == NULL) return;

    if (symbol->symbolType == program) {
        analyze_ast(node->left, tab);
        analyze_ast(node->right, tab);
        return;
    }

    switch (symbol->symbolType) {
        Symbol *existing;
        case(varDeclaration):   {
            existing = findSymbol(symbol, tab);
                if (existing != NULL && existing->symbolType == symbol->symbolType) {
                    printf("Error Semántico: La variable '%s' ya fue declarada.\n", symbol->id);
                    exit(1);
                } else {
                    addSymbolToTab(symbol, tab);
                }
            break;
        }
        case (methodDeclaration):{
                existing = findSymbol(symbol, tab);
            if (existing != NULL) {
                printf("Error Semántico: La función '%s' ya está definida.\n", symbol->id);
                exit(1);
            } else {
                addSymbolToTab(symbol, tab);
            }
            current_method_return_type = node->symbolData->type;

            openLevel(tab); 
            
            analyze_ast(node->left, tab); 
            analyze_ast(node->right, tab); 
            
            closeLevel(tab); 
            break;
        }

        case (block): {
            openLevel(tab);
            analyze_ast(node->left, tab);
            analyze_ast(node->right, tab);
            closeLevel(tab);
            break;
        }
        case ifNode:   {
            enum types cond_type = analyze_expression(node->left, tab);
            if (cond_type != tBoolean){
                printf("Error Semántico: La condición del IF debe ser de tipo booleano.\n");
                exit(1);
            }
        
            analyze_ast(node->right, tab); 
            break;
        }
        case whileNode: {
            enum types cond_type = analyze_expression(node->left, tab);
            if (cond_type != tBoolean){
                printf("Error Semántico: La condición del WHILE debe ser de tipo booleano.\n");
                exit(1);
            }
            
            analyze_ast(node->right, tab); 
            break;
        }

        case returnNode: { 
            enum types ret_type = tVoid; 
            if (node->left != NULL) {
                ret_type = analyze_expression(node->left, tab);
            }

            if (ret_type != current_method_return_type) {
                printf("Error Semántico: Tipo de retorno incorrecto en la función.\n");
                exit(1);
            }
            break;
        }
        case (assingOp):
            Symbol* leftBranch = node->left->symbolData;
            Symbol* rightBranch = node->right->symbolData;

            if (leftBranch->type == rightBranch->type){
                analyze_expression(node->left, tab);
                analyze_expression(node->right, tab);
            }else{
                printf("Error Semántico: Incompatibilidad de tipos en la asignación.\n");
                exit(1);
            }
    }

}