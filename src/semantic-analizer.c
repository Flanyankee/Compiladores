#include "semantic-analizer.h"
#include "constants.h"
#include "symbols-tab.h"
#include <stdio.h>
#include <string.h>

FILE *semanticOutputFile = NULL;
static enum types currentMethodReturnType;
int semanticErrorFlag = 0;

int analyzeAst(ASTNode *node, SymbolTab *tab) {
	if (node == NULL || node->symbolData == NULL)
		return semanticErrorFlag;

	Symbol *symbol = node->symbolData;
	int err = 0;

	switch (symbol->symbolType) {

	case program: {
		openLevel(tab);
		err |= analyzeAst(node->left, tab);
		err |= analyzeAst(node->right, tab);

		Symbol query = {.id = "main"};
		Symbol *mainFunc = findSymbol(&query, tab);

		if (mainFunc == NULL || mainFunc->symbolType != function) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Program must contain a 'main' function.\n",
                    symbol->line);
			return 1;
		}

		if (mainFunc->paramsNode != NULL) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): The 'main' function must take no parameters.\n",
                    mainFunc->line);
			return 1;
		}
		return err | semanticErrorFlag;
	}

	case varDeclaration: {
		ASTNode *currentDecl = node;
		while (currentDecl != NULL &&
		       currentDecl->symbolData->symbolType == varDeclaration) {
			ASTNode *varNode = currentDecl->left;
			if (varNode != NULL &&
			    varNode->symbolData->symbolType == variable) {
				Symbol *sym = varNode->symbolData;

				if (findSymbol(sym, tab) != NULL) {
					fprintf(semanticOutputFile,
                            "Semantic Error (Line %d): The identifier '%s' was already declared in this scope.\n",
                            sym->line, sym->id);
					semanticErrorFlag = 1;
					return 1;
				}

				if ((tab->firstLevel)->up == (tab->lastLevel->down)) {
					sym->symbolType = global_var;
				} else {
					sym->symbolType = local_var;
				}
				addSymbolToTab(sym, tab);
			}
			currentDecl = currentDecl->right;
		}
		return semanticErrorFlag;
	}

	case function: {
		Symbol *existing = findSymbol(symbol, tab);

		if (existing != NULL) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): The function '%s' is already defined.\n",
                    symbol->line, symbol->id);
			semanticErrorFlag = 1;
			return 1;
		}

		symbol->paramsNode = node->left;

		int param_count = 0;
		ASTNode *currentParam = node->left;
		while (currentParam != NULL &&
		       currentParam->symbolData->symbolType == formalParameterList) {
			param_count++;
			currentParam = currentParam->right;
		}

		if (strcmp(symbol->id, "main") == 0 && param_count != 0) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): The 'main' function must take no parameters.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return 1;
		}

		addSymbolToTab(symbol, tab);
		currentMethodReturnType = symbol->type;

		openLevel(tab);

		currentParam = node->left;
		while (currentParam != NULL &&
		       currentParam->symbolData->symbolType == formalParameterList) {
			ASTNode *paramNode = currentParam->left;
			if (paramNode != NULL &&
			    paramNode->symbolData->symbolType == formalParameter) {
				Symbol *paramSym = paramNode->symbolData;
				if (findSymbol(paramSym, tab) != NULL) {
					fprintf(semanticOutputFile,
                            "Semantic Error (Line %d): Duplicate parameter '%s'.\n",
                            paramSym->line, paramSym->id);
					semanticErrorFlag = 1;
					closeLevel(tab);
					return 1;
				}
				addSymbolToTab(paramSym, tab);
			}
			currentParam = currentParam->right;
		}

		if (node->right != NULL) {
			err |= analyzeAst(node->right, tab);
		}
		closeLevel(tab);
		return err | semanticErrorFlag;
	}

	case block: {
		openLevel(tab);
		if (node->left)
			err |= analyzeAst(node->left, tab);
		if (node->right)
			err |= analyzeAst(node->right, tab);
		closeLevel(tab);
		return err | semanticErrorFlag;
	}

	case ifNode: {
		enum types cond_type = analyzeExpression(node->left, tab);
		if (cond_type != tBoolean) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): The IF condition must be boolean.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			err = 1;
		}
		if (node->right)
			err |= analyzeAst(node->right, tab);
		return err | semanticErrorFlag;
	}

	case elseNode: {
		if (node->left)
			err |= analyzeAst(node->left, tab);
		if (node->right)
			err |= analyzeAst(node->right, tab);
		return err | semanticErrorFlag;
	}

	case whileNode: {
		enum types cond_type = analyzeExpression(node->left, tab);
		if (cond_type != tBoolean) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): The WHILE condition must be boolean.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			err = 1;
		}
		if (node->right)
			err |= analyzeAst(node->right, tab);
		return err | semanticErrorFlag;
	}

	case returnNode: {
		if (currentMethodReturnType == tVoid) {
			if (node->left != NULL) {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): A void method cannot return an expression.\n",
                        symbol->line);
                semanticErrorFlag = 1;
				return 1;
			}
		} else {
			if (node->left == NULL) {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): A non-void method must return a value.\n",
                        symbol->line);
                semanticErrorFlag = 1;
				return 1;
			}

			enum types ret_type = analyzeExpression(node->left, tab);

			if (ret_type != currentMethodReturnType) {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): Incompatible return type. Expected %d, got %d.\n",
                        symbol->line, currentMethodReturnType, ret_type);
                semanticErrorFlag = 1;
				return 1;
			}
		}
		return semanticErrorFlag;
	}

	case assignOp: {
		if (node->left == NULL || node->left->symbolData == NULL)
			return semanticErrorFlag;

		Symbol *varSym = findSymbolMultiLevel(node->left->symbolData, tab);
		if (varSym == NULL) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Variable '%s' not declared.\n",
                    node->left->symbolData->line, node->left->symbolData->id);
			semanticErrorFlag = 1;
			return 1;
		}

		enum types left_type = varSym->type;
		enum types right_type = analyzeExpression(node->right, tab);

		if (left_type == tVoid || right_type == tVoid) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Incompatible types in assignment involving 'void'.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return 1;
		}

		if (left_type != right_type) {
			if (left_type == tFloat && right_type == tInt) {
			} else if (left_type == tInt && right_type == tFloat) {
				fprintf(semanticOutputFile,
                        "Semantic Warning (Line %d): Possible loss of precision when assigning float to int.\n",
                        symbol->line);
			} else {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): Type mismatch in assignment.\n",
                        symbol->line);
				semanticErrorFlag = 1;
				return 1;
			}
		}
		return semanticErrorFlag;
	}

	case functionInvoke: {
		evaluateFunctionInvoke(node, tab, 0);
		return semanticErrorFlag;
	}

	default:
		if (node->left)
			err |= analyzeAst(node->left, tab);
		if (node->right)
			err |= analyzeAst(node->right, tab);
		return err | semanticErrorFlag;
	}
}

enum types analyzeExpression(ASTNode *expr, SymbolTab *tab) {
	if (expr == NULL || expr->symbolData == NULL)
		return tVoid;

	Symbol *symbol = expr->symbolData;

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
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Identifier '%s' not declared.\n",
                    symbol->line, symbol->id);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return sym->type;
	}

	case functionInvoke: {
		return evaluateFunctionInvoke(expr, tab, 1);
	}

	case orOp:
	case andOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left != tBoolean || right != tBoolean) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Logical operators require boolean operands.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	case negationOp: {
		enum types exp_type = analyzeExpression(expr->left, tab);
		if (exp_type != tBoolean) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Operator '!' requires a boolean operand.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	case addOp:
	case multOp:
	case divOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left == tBoolean || right == tBoolean || left == tVoid ||
		    right == tVoid) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Invalid arithmetic operation.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		if (left == tFloat || right == tFloat)
			return tFloat;
		return tInt;
	}

	case substractOp: {
		if (expr->right == NULL) {
			enum types exp_type = analyzeExpression(expr->left, tab);
			if (exp_type != tInt && exp_type != tFloat) {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): Unary '-' operator requires a numeric operand.\n",
                        symbol->line);
				semanticErrorFlag = 1;
				return tVoid;
			}
			return exp_type;
		} else {
			enum types left = analyzeExpression(expr->left, tab);
			enum types right = analyzeExpression(expr->right, tab);
			if (left == tBoolean || right == tBoolean || left == tVoid ||
			    right == tVoid) {
				fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): Invalid arithmetic operation.\n",
                        symbol->line);
				semanticErrorFlag = 1;
				return tVoid;
			}
			if (left == tFloat || right == tFloat)
				return tFloat;
			return tInt;
		}
	}

	case modOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left != tInt || right != tInt) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Modulo ('%%') operator requires integer operands.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tInt;
	}

	case lessOp:
	case greaterOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if ((left != tInt && left != tFloat) ||
		    (right != tInt && right != tFloat)) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Relational comparison requires numeric types.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	case equalsOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left == tVoid || right == tVoid) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Equality comparison involving 'void' type.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		if (left != right && !((left == tInt && right == tFloat) || (left == tFloat && right == tInt))) {
			fprintf(semanticOutputFile,
                    "Semantic Error (Line %d): Equality comparison between incompatible types.\n",
                    symbol->line);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	default:
		return expr->symbolData->type;
	}
}

static enum types evaluateFunctionInvoke(ASTNode *expr, SymbolTab *tab,
                                         int isExpression) {
	if (expr == NULL || expr->symbolData == NULL)
		return tVoid;

	Symbol *callSym = expr->symbolData;
	Symbol *func = findSymbolMultiLevel(expr->symbolData, tab);
	
	if (func == NULL || func->symbolType != function) {
		fprintf(semanticOutputFile,
                "Semantic Error (Line %d): Function '%s' not declared.\n",
                callSym->line, callSym->id);
        semanticErrorFlag = 1;
		return tVoid;
	}

	if (isExpression && func->type == tVoid) {
		fprintf(semanticOutputFile,
                "Semantic Error (Line %d): Method '%s' does not return a result and cannot be used as an expression.\n",
                callSym->line, func->id);
        semanticErrorFlag = 1;
	}

	ASTNode *currActual = expr->left;
	ASTNode *currFormal = func->paramsNode;
	int arg_index = 1;

	while (currActual != NULL && currFormal != NULL) {
		enum types actual_type = analyzeExpression(currActual->left, tab);

		ASTNode *formalNode = currFormal->left;
		enum types formal_type = formalNode->symbolData->type;

		if (actual_type != formal_type) {
			if (!(formal_type == tFloat && actual_type == tInt)) {
				int errLine = (currActual->left && currActual->left->symbolData)
                                  ? currActual->left->symbolData->line
                                  : callSym->line;
                fprintf(semanticOutputFile,
                        "Semantic Error (Line %d): In function '%s', parameter %d expected type %d, but got type %d.\n",
                        errLine, func->id, arg_index, formal_type, actual_type);
                semanticErrorFlag = 1;
			}
		}

		arg_index++;
		currActual = currActual->right;
		currFormal = currFormal->right;
	}

	if (currActual != NULL || currFormal != NULL) {
		fprintf(semanticOutputFile,
                "Semantic Error (Line %d): Function '%s' argument count mismatch.\n",
                callSym->line, func->id);
        semanticErrorFlag = 1;
	}

	return func->type;
}
