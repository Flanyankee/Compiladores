#include "semantic-analizer.h"
#include "constants.h"
#include "symbols-tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static enum types current_method_return_type; 

void analyze_ast(ASTNode *node, SymbolTab *tab) {
    if (node == NULL || node->symbolData == NULL) return;

    Symbol* symbol = node->symbolData;

    switch (symbol->symbolType) {
        case program:
        case methodDeclarationList:
        case varDeclarationList:
        case statementList:
            // Nodos puramente estructurales que bajan por el árbol
            analyze_ast(node->left, tab);
            analyze_ast(node->right, tab);
            break;

        case varDeclaration: {
            // El hijo izquierdo es un idList con las variables declaradas
            ASTNode *currentIdList = node->left;
            while (currentIdList != NULL && currentIdList->symbolData->symbolType == idList) {
                ASTNode *varNode = currentIdList->left;
                if (varNode != NULL && varNode->symbolData->symbolType == variable) {
                    Symbol *sym = varNode->symbolData;
                    
                    if (findSymbol(sym, tab) != NULL) {
                        printf("Semantic Error: The identifier '%s' was already declared in this scope.\n", sym->id);
                        exit(1);
                    }

                    if ((tab->firstLevel)->up == (tab->lastLevel->down)) {
                        sym->symbolType = global_var;
                    } else {
                        sym->symbolType = local_var;
                    }
                    addSymbolToTab(sym, tab);
                }
                currentIdList = currentIdList->right; // Avanzar en la lista
            }
            break;
        }

        case function: { // El parser usa 'function' para los method_decl
            Symbol *existing = findSymbol(symbol, tab);
            if (existing != NULL) {
                printf("Semantic Error: The function '%s' is already defined.\n", symbol->id);
                exit(1);
            }
            
            // 1. Contar la cantidad de parámetros para futuras invocaciones
            int param_count = 0;
            ASTNode *currentParam = node->left;
            while (currentParam != NULL && currentParam->symbolData->symbolType == formalParameterList) {
                param_count++;
                currentParam = currentParam->right;
            }
            symbol->value.intValue = param_count; // Guardamos en la union
            symbol->hasValue = 1;
            
            addSymbolToTab(symbol, tab);
            current_method_return_type = symbol->type;
            
            openLevel(tab); 
            
            // 2. Insertar los parámetros formales en el nuevo ámbito local
            currentParam = node->left;
            while (currentParam != NULL && currentParam->symbolData->symbolType == formalParameterList) {
                ASTNode *paramNode = currentParam->left;
                if (paramNode != NULL && paramNode->symbolData->symbolType == formalParameter) {
                    Symbol *paramSym = paramNode->symbolData;
                    if (findSymbol(paramSym, tab) != NULL) {
                        printf("Semantic Error: Duplicate parameter '%s'.\n", paramSym->id);
                        exit(1);
                    }
                    addSymbolToTab(paramSym, tab);
                }
                currentParam = currentParam->right;
            }
            
            // 3. Analizar el cuerpo de la función (bloque)
            if (node->right != NULL) {
                analyze_ast(node->right, tab); 
            }
            closeLevel(tab); 
            break;
        }

        case block: {
            openLevel(tab);
            analyze_ast(node->left, tab);
            analyze_ast(node->right, tab);
            closeLevel(tab);
            break;
        }

        case ifNode: {
            enum types cond_type = analyze_expression(node->left, tab);
            if (cond_type != tBoolean) {
                printf("Semantic Error: The IF condition must be boolean.\n");
                exit(1);
            }
            analyze_ast(node->right, tab); // Analizar cuerpo y posible ELSE anidado
            break;
        }

        case elseNode: {
            analyze_ast(node->left, tab);  
            analyze_ast(node->right, tab);
            break;
        }

        case whileNode: {
            enum types cond_type = analyze_expression(node->left, tab);
            if (cond_type != tBoolean) {
                printf("Semantic Error: The WHILE condition must be boolean.\n");
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
                // Permitimos promoción implícita de int a float
                if (!(current_method_return_type == tFloat && ret_type == tInt)) {
                    printf("Semantic Error: Incompatible return type in '%s'.\n", symbol->id ? symbol->id : "");
                    exit(1);
                }
            }
            break;
        }

        case assignOp: {
            // Verificación defensiva contra árboles incompletos
            if (node->left == NULL || node->left->symbolData == NULL) break;
            
            // Buscar la variable destino (ya debe estar declarada)
            Symbol *varSym = findSymbolMultiLevel(node->left->symbolData, tab);
            if (varSym == NULL) {
                printf("Semantic Error: Variable '%s' not declared.\n", node->left->symbolData->id);
                exit(1);
            }
            
            enum types left_type = varSym->type;
            enum types right_type = analyze_expression(node->right, tab);

            if (left_type == tVoid || right_type == tVoid) {
                printf("Semantic Error: Incompatible types in assignment involving 'void'.\n");
                exit(1);
            }
            
            if (left_type != right_type) {
                if (left_type == tFloat && right_type == tInt) {
                    // Promoción de int a float es válida
                } else if (left_type == tInt && right_type == tFloat) {
                    printf("Semantic Warning: Possible loss of precision when assigning float to int.\n");
                } else {
                    printf("Semantic Error: Type mismatch in assignment.\n");
                    exit(1);
                }
            }
            break;
        }

        case functionInvoke: {
            // Maneja invocaciones sueltas (statement -> method_call SEMICOLON)
            analyze_expression(node, tab);
            break;
        }

        default:
            analyze_ast(node->left, tab);
            analyze_ast(node->right, tab);
            break;
    }
}

enum types analyze_expression(ASTNode *expr, SymbolTab *tab) {
    if (expr == NULL || expr->symbolData == NULL) return tVoid;

    switch (expr->symbolData->symbolType) {
        case intLiteral:
        case floatLiteral:
        case boolLiteral:
            return expr->symbolData->type;

        case variable:
        case global_var: 
        case local_var:
        case formalParameter: {
            Symbol *sym = findSymbolMultiLevel(expr->symbolData, tab);
            if (sym == NULL) {
                printf("Semantic Error: Identifier '%s' not declared.\n", expr->symbolData->id);
                exit(1);
            }
            return sym->type;
        }

        case functionInvoke: { 
            Symbol *func = findSymbolMultiLevel(expr->symbolData, tab);
            if (func == NULL || func->symbolType != function) {
                printf("Semantic Error: Function '%s' not declared.\n", expr->symbolData->id);
                exit(1);
            }

            // Recorrido de los parámetros enviados (actualParameterList)
            ASTNode *argList = expr->left; 
            int arg_count = 0;
            while (argList != NULL && argList->symbolData->symbolType == actualParameterList) {
                analyze_expression(argList->left, tab); // Validar la expresión enviada
                arg_count++;
                argList = argList->right;
            }

            if (func->hasValue && func->value.intValue != arg_count) {
                printf("Semantic Error: Function '%s' expected %d arguments, received %d.\n", 
                        func->id, func->value.intValue, arg_count);
                exit(1);
            }
            return func->type;
        }

        case orOp:
        case andOp: {
            enum types left = analyze_expression(expr->left, tab);
            enum types right = analyze_expression(expr->right, tab);
            if (left != tBoolean || right != tBoolean) {
                printf("Semantic Error: Logical operators require boolean operands.\n");
                exit(1);
            }
            return tBoolean;
        }

        case negationOp: { // Creado por EXCLAMATION expr
            enum types exp_type = analyze_expression(expr->left, tab);
            if (exp_type != tBoolean) {
                printf("Semantic Error: Operator '!' requires a boolean operand.\n");
                exit(1);
            }
            return tBoolean;
        }

        case addOp:
        case multOp:
        case divOp: {
            enum types left = analyze_expression(expr->left, tab);
            enum types right = analyze_expression(expr->right, tab);
            if (left == tBoolean || right == tBoolean || left == tVoid || right == tVoid) {
                printf("Semantic Error: Invalid arithmetic operation.\n");
                exit(1);
            }
            if (left == tFloat || right == tFloat) return tFloat;
            return tInt;
        }

        case substractOp: {
            // El parser reutiliza substractOp para unarios (right == NULL) y binarios (ambos con valor)
            if (expr->right == NULL) { 
                enum types exp_type = analyze_expression(expr->left, tab);
                if (exp_type != tInt && exp_type != tFloat) {
                    printf("Semantic Error: Unary '-' operator requires a numeric operand.\n");
                    exit(1);
                }
                return exp_type;
            } else {
                enum types left = analyze_expression(expr->left, tab);
                enum types right = analyze_expression(expr->right, tab);
                if (left == tBoolean || right == tBoolean || left == tVoid || right == tVoid) {
                    printf("Semantic Error: Invalid arithmetic operation.\n");
                    exit(1);
                }
                if (left == tFloat || right == tFloat) return tFloat;
                return tInt;
            }
        }

        case modOp: {
            enum types left = analyze_expression(expr->left, tab);
            enum types right = analyze_expression(expr->right, tab);
            if (left != tInt || right != tInt) {
                printf("Semantic Error: Modulo ('%%') operator requires integer operands.\n");
                exit(1);
            }
            return tInt;
        }

        case lessOp:
        case greaterOp: {
            enum types left = analyze_expression(expr->left, tab);
            enum types right = analyze_expression(expr->right, tab);
            if ((left != tInt && left != tFloat) || (right != tInt && right != tFloat)) {
                printf("Semantic Error: Relational comparison requires numeric types.\n");
                exit(1);
            }
            return tBoolean;
        }

        case equalsOp: {
            enum types left = analyze_expression(expr->left, tab);
            enum types right = analyze_expression(expr->right, tab);
            if (left == tVoid || right == tVoid) {
                printf("Semantic Error: Equality comparison involving 'void' type.\n");
                exit(1);
            }
            if (left != right && !((left == tInt && right == tFloat) || (left == tFloat && right == tInt))) {
                printf("Semantic Error: Equality comparison between incompatible types.\n");
                exit(1);
            }
            return tBoolean;
        }

        default:
            return expr->symbolData->type;
    }
}