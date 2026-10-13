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
		err |= analyzeAst(node->left, tab);
		err |= analyzeAst(node->right, tab);

		Symbol query = {.id = "main"};
		Symbol *mainFunc = findSymbol(&query, tab);

		if (mainFunc == NULL || mainFunc->symbolType != function) {
			fprintf(semanticOutputFile,"Semantic Error: Program must contain a 'main' function.\n");
			semanticErrorFlag = 1;
			return 1;
		}

		if (mainFunc->paramsNode != NULL) {
			fprintf(semanticOutputFile,"Semantic Error: The 'main' function must take no parameters.\n");
			semanticErrorFlag = 1;
			return 1;
		}
		return err | semanticErrorFlag;
	}

	case varDeclaration: {
		ASTNode *currentVar = node->left;
		while (currentVar != NULL &&
		       currentVar->symbolData->symbolType == variable) {
			ASTNode *varNode = currentVar->left;
			if (varNode != NULL &&
			    varNode->symbolData->symbolType == variable) {
				Symbol *sym = varNode->symbolData;

				if (findSymbol(sym, tab) != NULL) {
					fprintf(semanticOutputFile,"Semantic Error: The identifier '%s' was already declared in this scope.\n",sym->id);
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
			currentVar = currentVar->right;
		}
		return semanticErrorFlag;
	}

	case function: {
		Symbol *existing = findSymbol(symbol, tab);

		if (existing != NULL) {
			fprintf(semanticOutputFile,"Semantic Error: The function '%s' is already defined.\n", symbol->id);
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
			fprintf(semanticOutputFile,"Semantic Error: The 'main' function must take no parameters.\n");
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
					fprintf(semanticOutputFile,"Semantic Error: Duplicate parameter '%s'.\n", paramSym->id);
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
			fprintf(semanticOutputFile,"Semantic Error: The IF condition must be boolean.\n");
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
			fprintf(semanticOutputFile,"Semantic Error: The WHILE condition must be boolean.\n");
			semanticErrorFlag = 1;
			err = 1;
		}
		if (node->right)
			err |= analyzeAst(node->right, tab);
		return err | semanticErrorFlag;
	}

	case returnNode: {
		enum types ret_type = tVoid;
		if (node->left != NULL) {
			ret_type = analyzeExpression(node->left, tab);
		}

		if (ret_type != currentMethodReturnType) {
			if (!(currentMethodReturnType == tFloat && ret_type == tInt)) {
				fprintf(semanticOutputFile,"Semantic Error: Incompatible return type in '%s'.\n", symbol->id ? symbol->id : "");
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
			fprintf(semanticOutputFile,"Semantic Error: Variable '%s' not declared.\n", node->left->symbolData->id);
			semanticErrorFlag = 1;
			return 1;
		}

		enum types left_type = varSym->type;
		enum types right_type = analyzeExpression(node->right, tab);

		if (left_type == tVoid || right_type == tVoid) {
			fprintf(semanticOutputFile,"Semantic Error: Incompatible types in assignment involving 'void'.\n");
			semanticErrorFlag = 1;
			return 1;
		}

		if (left_type != right_type) {
			if (left_type == tFloat && right_type == tInt) {
			} else if (left_type == tInt && right_type == tFloat) {
				fprintf(semanticOutputFile,"Semantic Warning: Possible loss of precision when assigning float to int.\n");
			} else {
				fprintf(semanticOutputFile,"Semantic Error: Type mismatch in assignment.\n");
				semanticErrorFlag = 1;
				return 1;
			}
		}
		return semanticErrorFlag;
	}

	case functionInvoke: {
		analyzeExpression(node, tab);
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
			fprintf(semanticOutputFile,"Semantic Error: Identifier '%s' not declared.\n", expr->symbolData->id);
			semanticErrorFlag = 1;
			return tVoid;
		}
		return sym->type;
	}

	case functionInvoke: {
		Symbol *func = findSymbolMultiLevel(expr->symbolData, tab);
		if (func == NULL || func->symbolType != function) {
			fprintf(semanticOutputFile,"Semantic Error: Function '%s' not declared.\n", expr->symbolData->id);
			semanticErrorFlag = 1;
			return tVoid;
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
					fprintf(semanticOutputFile,"Semantic Error: In function '%s', parameter %d expected type %d, but got type %d.\n",
					    func->id, arg_index, formal_type, actual_type);
					semanticErrorFlag = 1;
				}
			}

			arg_index++;
			currActual = currActual->right;
			currFormal = currFormal->right;
		}

		if (currActual != NULL || currFormal != NULL) {
			fprintf(semanticOutputFile,"Semantic Error: Function '%s' argument count mismatch.\n", func->id);
			semanticErrorFlag = 1;
		}

		return func->type;
	}

	case orOp:
	case andOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left != tBoolean || right != tBoolean) {
			fprintf(semanticOutputFile,"Semantic Error: Logical operators require boolean operands.\n");
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	case negationOp: {
		enum types exp_type = analyzeExpression(expr->left, tab);
		if (exp_type != tBoolean) {
			fprintf(semanticOutputFile,"Semantic Error: Operator '!' requires a boolean operand.\n");
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
			fprintf(semanticOutputFile,"Semantic Error: Invalid arithmetic operation.\n");
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
				fprintf(semanticOutputFile,"Semantic Error: Unary '-' operator requires a numeric operand.\n");
				semanticErrorFlag = 1;
				return tVoid;
			}
			return exp_type;
		} else {
			enum types left = analyzeExpression(expr->left, tab);
			enum types right = analyzeExpression(expr->right, tab);
			if (left == tBoolean || right == tBoolean || left == tVoid ||
			    right == tVoid) {
				fprintf(semanticOutputFile,"Semantic Error: Invalid arithmetic operation.\n");
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
			fprintf(semanticOutputFile,"Semantic Error: Modulo ('%%') operator requires integer operands.\n");
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
			fprintf(semanticOutputFile,"Semantic Error: Relational comparison requires numeric types.\n");
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	case equalsOp: {
		enum types left = analyzeExpression(expr->left, tab);
		enum types right = analyzeExpression(expr->right, tab);
		if (left == tVoid || right == tVoid) {
			fprintf(semanticOutputFile,"Semantic Error: Equality comparison involving 'void' type.\n");
			semanticErrorFlag = 1;
			return tVoid;
		}
		if (left != right && !((left == tInt && right == tFloat) ||
		                       (left == tFloat && right == tInt))) {
			fprintf(semanticOutputFile,"Semantic Error: Equality comparison between incompatible types.\n");
			semanticErrorFlag = 1;
			return tVoid;
		}
		return tBoolean;
	}

	default:
		return expr->symbolData->type;
	}
}
