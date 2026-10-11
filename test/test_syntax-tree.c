#include "constants.h"
#include "lex.yy.c"
#include "parser.tab.h"
#include "syntax-tree.h"
#include "unity.h"
#include <stdlib.h>

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
	ASTNode *yNode = ((intVarDeclarationNode)->right)->left;

	ASTNode *floatVarDeclarationNode = varDeclarationListNode->right;
	ASTNode *rNode = (floatVarDeclarationNode)->left;
	ASTNode *sNode = ((floatVarDeclarationNode)->right)->left;

	TEST_ASSERT_EQUAL(varDeclarationList,
	                  (varDeclarationListNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(varDeclaration,
	                  (intVarDeclarationNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(varDeclaration,
	                  (floatVarDeclarationNode->symbolData)->symbolType);

	TEST_ASSERT_EQUAL(variable, (xNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tInt, (xNode->symbolData)->type);
	TEST_ASSERT_EQUAL(variable, (yNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tInt, (yNode->symbolData)->type);

	TEST_ASSERT_EQUAL(variable, (rNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tFloat, (rNode->symbolData)->type);
	TEST_ASSERT_EQUAL(variable, (sNode->symbolData)->symbolType);
	TEST_ASSERT_EQUAL(tFloat, (sNode->symbolData)->type);
}
