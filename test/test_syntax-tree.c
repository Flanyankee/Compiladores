#include "constants.h"
#include "lex.yy.c"
#include "parser.tab.h"
#include "syntax-tree.h"
#include "unity.h"
#include <stdlib.h>
#include <unistd.h>

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);
extern int yyparse(void);

FILE *outputFilePtr = NULL;

void setUp(void) {}

void tearDown(void) {}

static int parse_string(const char *input) {
	YY_BUFFER_STATE buffer = yy_scan_string(input);
	int result = yyparse();
	yy_delete_buffer(buffer);
	return result;
}

void test_empty_tree(void) {
	const char *code = "";
	int result = parse_string(code);
	TEST_ASSERT_EQUAL(0, result);
	TEST_ASSERT_EQUAL(program, (syntaxTree->symbolData)->symbolType);
}

void test_var_declaration() {
	const char *code = "int x, y; float r, s;";
	int result = parse_string(code);
	TEST_ASSERT_EQUAL(0, result);

	ASTNode *varDeclarationListNode = syntaxTree->left;

	ASTNode *intVarDeclarationNode = (varDeclarationListNode->left)->right;
	ASTNode *xNode = (intVarDeclarationNode)->left;

	ASTNode *floatVarDeclarationNode = varDeclarationListNode->right;
	ASTNode *sNode = ((floatVarDeclarationNode)->right)->left;

	TEST_ASSERT_EQUAL(varDeclarationList,
	                  (varDeclarationListNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(varDeclaration,
	                  (intVarDeclarationNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(varDeclaration,
	                  (floatVarDeclarationNode->symbolData)->symbolType);

	TEST_ASSERT_EQUAL(variable, (xNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tInt, (xNode->symbolData)->type);
	TEST_ASSERT_EQUAL_STRING("x", (xNode->symbolData)->id);
	TEST_ASSERT_EQUAL(0, (xNode->symbolData)->hasValue);

	TEST_ASSERT_EQUAL(variable, (sNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tFloat, (sNode->symbolData)->type);
}

void test_method_declaration() {
	const char *code = "void main(int x2, int x3) {int x;}";
	int result = parse_string(code);
	TEST_ASSERT_EQUAL(0, result);

	ASTNode *methodDeclarationNode = (syntaxTree->right)->left;
	ASTNode *parametersListNode = (methodDeclarationNode->left);
	ASTNode *blockNode = (methodDeclarationNode->right);

	TEST_ASSERT_EQUAL(function,
	                  (methodDeclarationNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tVoid, (methodDeclarationNode->symbolData)->type);
	TEST_ASSERT_EQUAL_STRING("main", (methodDeclarationNode->symbolData)->id);

	TEST_ASSERT_EQUAL(formalParameterList,
	                  (parametersListNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(block, (blockNode->symbolData)->symbolType);

	ASTNode *x3Node = ((parametersListNode->right)->left);
	TEST_ASSERT_EQUAL(formalParameter, (x3Node->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tInt, (x3Node->symbolData)->type);
	TEST_ASSERT_EQUAL_STRING("x3", (x3Node->symbolData)->id);
	TEST_ASSERT_EQUAL(0, (x3Node->symbolData)->hasValue);
}

void test_statement_assign() {
	const char *code = "void main(int x2, int x3) {x2 = 1.5;}";
	int result = parse_string(code);
	TEST_ASSERT_EQUAL(0, result);

	ASTNode *methodDeclarationNode = (syntaxTree->right)->left;
	ASTNode *blockNode = (methodDeclarationNode->right);
	ASTNode *assignNode = (blockNode->right)->left;

	TEST_ASSERT_EQUAL(assignOp, (assignNode->symbolData)->symbolType);

	ASTNode *idAssign = assignNode->left, *exprAssing = assignNode->right;
	TEST_ASSERT_EQUAL_STRING("x2", (idAssign->symbolData)->id);
	TEST_ASSERT_EQUAL(floatLiteral, (exprAssing->symbolData)->symbolType);
}
