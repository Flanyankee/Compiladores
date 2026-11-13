#include "optimizations.h"
#include "constants.h"
#include "syntax-tree.h"
#include <stddef.h>

void constantOptimizations(void) {
	syntaxTree = constantOptimizationsHelper(syntaxTree);
}

void deadCodeOptimizations(void) {
    syntaxTree = optimizeUnreachableCode(syntaxTree);
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
		                     lSym->symbolType == floatLiteral);
		int isRightLiteral = (rSym->symbolType == intLiteral ||
		                      rSym->symbolType == floatLiteral);

		if (isLeftLiteral && isRightLiteral) {
			return foldConstantOperation(node, lSym, rSym);
		}
	}

	return node;
}

Symbol *createLiteralSymbol(int isFloat, float resFloat, int resInt) {
	Symbol *result = NULL;

	if (isFloat) {
		result = makeSymbol("", floatLiteral);
		result->type = tFloat;
		result->value.floatValue = resFloat;
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

	float leftFloat = (leftSym->type == tFloat)
	                      ? leftSym->value.floatValue
	                      : (float)leftSym->value.intValue;
	float rightFloat = (rightSym->type == tFloat)
	                       ? rightSym->value.floatValue
	                       : (float)rightSym->value.intValue;

	int leftInt = leftSym->value.intValue;
	int rightInt = rightSym->value.intValue;

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
		resFloat = leftFloat / rightFloat;
		resInt = leftInt / rightInt;
		break;
	case modOp:
		resInt = leftInt % rightInt;
		break;
	default:
		return node;
	}

	Symbol *resultSymbol = createLiteralSymbol(hasFloat, resFloat, resInt);
	return makeLeaf(resultSymbol);
}
ASTNode* optimizeUnreachableCode(ASTNode* node) {
    if (node == NULL) return NULL;

    node->left = optimizeUnreachableCode(node->left);
    node->right = optimizeUnreachableCode(node->right);

    if (node->symbolData->symbolType == statementList) {
        ASTNode* currentStatement = node->left;
        if (currentStatement != NULL && currentStatement->symbolData != NULL && 
            currentStatement->symbolData->symbolType == returnNode) {

            node->right = NULL;
        }
    }

    if (node->symbolData->symbolType == ifNode || node->symbolData->symbolType == whileNode) {
        ASTNode* condition = node->left;

        if (condition != NULL && condition->symbolData->symbolType == boolLiteral ) {
            if (condition->symbolData->value.boolValue == 0) {
                node = NULL;
                return NULL; 
            }
        }
    }
    
    return node;
}