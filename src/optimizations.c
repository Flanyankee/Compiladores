#include "optimizations.h"
#include "constants.h"
#include "syntax-tree.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void constantOptimizations(enum optimizations optLevel) {
	if (optLevel == constants || optLevel == all) {
		syntaxTree = constantOptimizationsHelper(syntaxTree);
	}
}

void deadCodeOptimizations(enum optimizations optLevel) {
	if (optLevel == dead_code || optLevel == all) {
		syntaxTree = optimizeUnreachableCode(syntaxTree);
	}
}

ASTNode *constantOptimizationsHelper(ASTNode *node) {
	if (node == NULL) {
		return NULL;
	}

	node->left = constantOptimizationsHelper(node->left);
	node->right = constantOptimizationsHelper(node->right);

	if (node->left != NULL && node->right != NULL) {
		Symbol *lSym = node->left->symbolData;
		Symbol *rSym = node->right->symbolData;

		int isLeftLiteral = (lSym->symbolType == intLiteral ||
		                     lSym->symbolType == boolLiteral ||
		                     lSym->symbolType == floatLiteral);
		int isRightLiteral = (rSym->symbolType == intLiteral ||
		                      rSym->symbolType == boolLiteral ||
		                      rSym->symbolType == floatLiteral);

		if (isLeftLiteral && isRightLiteral) {
			return foldConstantOperation(node, lSym, rSym);
		}
	} else if (node->left != NULL &&
	               (node->symbolData)->symbolType == negationOp ||
	           (node->symbolData)->symbolType == substractOp) {

		Symbol *lSym = node->left->symbolData;
		if (lSym->symbolType == boolLiteral || lSym->symbolType == intLiteral) {
			Symbol *rSym = makeSymbol("", blankNode);
			return foldConstantOperation(node, lSym, rSym);
		}
	}

	return node;
}

Symbol *createLiteralSymbol(int isFloat, int isBool, float resFloat,
                            int resInt) {
	Symbol *result = NULL;

	if (isFloat) {
		result = makeSymbol("", floatLiteral);
		result->type = tFloat;
		result->value.floatValue = resFloat;
	} else if (isBool) {
		result = makeSymbol("", boolLiteral);
		result->type = tBoolean;
		result->value.boolValue = resInt;
	} else {
		result = makeSymbol("", intLiteral);
		result->type = tInt;
		result->value.intValue = resInt;
	}

	result->hasValue = 1;
	return result;
}

ASTNode *foldConstantOperation(ASTNode *node, Symbol *leftSym,
                               Symbol *rightSym) {

	enum symbolTypes op = node->symbolData->symbolType;
	int hasFloat = (leftSym->type == tFloat || rightSym->type == tFloat);
	int isBool = (leftSym->type == tBoolean || rightSym->type == tBoolean);

	float leftFloat = (leftSym->type == tFloat)
	                      ? leftSym->value.floatValue
	                      : (float)leftSym->value.intValue;
	float rightFloat = (rightSym->type == tFloat)
	                       ? rightSym->value.floatValue
	                       : (float)rightSym->value.intValue;

	int leftInt = (leftSym->type == tInt) ? leftSym->value.intValue
	                                      : leftSym->value.boolValue;
	int rightInt = (rightSym->type == tInt) ? rightSym->value.intValue
	                                        : rightSym->value.boolValue;

	float resFloat = 0.0f;
	int resInt = 0;

	switch (op) {
	case addOp:
		resFloat = leftFloat + rightFloat;
		resInt = leftInt + rightInt;
		break;
	case substractOp:
		resFloat = leftFloat - rightFloat;
		resInt = leftInt - rightInt;
		break;
	case multOp:
		resFloat = leftFloat * rightFloat;
		resInt = leftInt * rightInt;
		break;
	case divOp:
		if (rightFloat == 0.0 || rightInt == 0) {
			printf("Error: division by zero is not permited.\n");
			exit(1);
		}
		resFloat = leftFloat / rightFloat;
		resInt = leftInt / rightInt;
		break;
	case modOp:
		if (rightInt == 0) {
			printf("Error: mod by zero is not permited.\n");
			exit(1);
		}
		resInt = leftInt % rightInt;
		break;
	case lessOp:
		resInt = leftInt < rightInt;
		break;
	case greaterOp:
		resInt = leftInt > rightInt;
		break;
	case equalsOp:
		resInt = leftInt == rightInt;
		break;
	case andOp:
		resInt = leftInt && rightInt;
		break;
	case orOp:
		resInt = leftInt || rightInt;
		break;
	case negationOp:
		resInt = !leftInt;
		break;
	default:
		return node;
	}

	Symbol *resultSymbol =
	    createLiteralSymbol(hasFloat, isBool, resFloat, resInt);
	return makeLeaf(resultSymbol);
}
ASTNode *optimizeUnreachableCode(ASTNode *node) {
	if (node == NULL)
		return NULL;

	node->left = optimizeUnreachableCode(node->left);
	node->right = optimizeUnreachableCode(node->right);

	if (node->symbolData->symbolType == statementList) {
		ASTNode *currentStatement = node->left;
		if (currentStatement != NULL && currentStatement->symbolData != NULL &&
		    currentStatement->symbolData->symbolType == returnNode) {

			node->right = NULL;
		}
	}

	if (node->symbolData->symbolType == ifNode ||
	    node->symbolData->symbolType == whileNode) {
		ASTNode *condition = node->left;

		if (condition != NULL &&
		    condition->symbolData->symbolType == boolLiteral) {
			if (condition->symbolData->value.boolValue == 0) {
				node = NULL;
				return NULL;
			}
		}
	}
	return node;
}
