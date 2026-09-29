#include "syntax-tree.h"
#include "unity.h"
#include <stdlib.h>

void setUp(void) {}

void tearDown(void) {}

void test_empty_tree(void) {
	ASTNode *tree = (ASTNode *)malloc(sizeof(ASTNode));

	TEST_ASSERT_NULL(tree->left);
	TEST_ASSERT_NULL(tree->right);
	TEST_ASSERT_NULL(tree->symbolData);
}

void test_make_symbol() {
	Symbol *symbol = makeSymbol("test", block);

	TEST_ASSERT_EQUAL(block, symbol->symbolType);
	TEST_ASSERT_EQUAL(blank, symbol->type);
	TEST_ASSERT_EQUAL(0, symbol->value);
	TEST_ASSERT_EQUAL(0, symbol->hasValue);
	TEST_ASSERT_EQUAL_STRING("test", symbol->id);
}

void test_more_complex_tree() {
	ASTNode *tree = (ASTNode *)malloc(sizeof(ASTNode));
	ASTNode *leftSon = makeLeaf(makeSymbol("test1", block));
	ASTNode *leftSonSubtree = makeLeaf(makeSymbol("test2", actualParameter));
	ASTNode *subtree =
	    makeNode(makeSymbol("test_subree", ifNode), leftSonSubtree, NULL);

	tree->left = leftSon;
	tree->right = subtree;
	subtree->left = leftSonSubtree;

	TEST_ASSERT_EQUAL(subtree, tree->right);
	TEST_ASSERT_EQUAL(leftSonSubtree, (tree->right)->left);
	TEST_ASSERT_NOT_NULL((tree->right)->left);
	TEST_ASSERT_NULL((tree->right)->right);
	TEST_ASSERT_EQUAL(leftSonSubtree->symbolData,
	                  ((tree->right)->left)->symbolData);

	TEST_ASSERT_NULL((tree->left)->left);
	TEST_ASSERT_NULL((tree->left)->right);
}
