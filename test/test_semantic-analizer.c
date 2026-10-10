#ifdef TEST

#include "unity.h"
#include "semantic-analizer.h"
#include "syntax-tree.h"
#include "symbols-tab.h"
#include "constants.h"
#include <stdlib.h>
#include <string.h>

static SymbolTab *tab = NULL;

void setUp(void) {
    tab = initializeSymbolTab();
}

void tearDown(void) {
    if (tab != NULL) {
        while ((tab->lastLevel)->down != tab->firstLevel) {
            closeLevel(tab);
        }
        free(tab->firstLevel);
        free(tab->lastLevel);
        free(tab);
        tab = NULL;
    }
}

void test_analyze_expression_literal(void) {
    Symbol *sym = makeSymbol("5", intLiteral);
    sym->type = tInt;
    ASTNode *node = makeLeaf(sym);

    enum types res = analyze_expression(node, tab);
    TEST_ASSERT_EQUAL_INT(tInt, res);
}

void test_analyze_expression_variable_found(void) {
    openLevel(tab);
    
    Symbol *sym_decl = makeSymbol("my_var", variable);
    sym_decl->type = tFloat;
    addSymbolToTab(sym_decl, tab);

    Symbol *sym_use = makeSymbol("my_var", variable);
    ASTNode *node = makeLeaf(sym_use);

    enum types res = analyze_expression(node, tab);
    TEST_ASSERT_EQUAL_INT(tFloat, res);
}

void test_analyze_expression_binary_operation(void) {
    Symbol *sym_left = makeSymbol("10", intLiteral);
    sym_left->type = tInt;
    ASTNode *node_left = makeLeaf(sym_left);

    Symbol *sym_right = makeSymbol("20", intLiteral);
    sym_right->type = tInt;
    ASTNode *node_right = makeLeaf(sym_right);

    Symbol *sym_add = makeSymbol("+", addOp);
    ASTNode *node_add = makeNode(sym_add, node_left, node_right);

    enum types res = analyze_expression(node_add, tab);
    TEST_ASSERT_EQUAL_INT(tInt, res);
}

void test_analyze_ast_varDeclaration_global(void) {
    openLevel(tab);
    
    Symbol *sym_var = makeSymbol("global_x", variable);
    sym_var->type = tInt;
    ASTNode *varNode = makeLeaf(sym_var);

    Symbol *sym_list = makeSymbol("list", idList);
    ASTNode *listNode = makeNode(sym_list, varNode, NULL);

    Symbol *sym_decl = makeSymbol("decl", varDeclaration);
    ASTNode *declNode = makeNode(sym_decl, listNode, NULL);

    analyze_ast(declNode, tab);

    Symbol query = {.id = "global_x"};
    Symbol *found = findSymbol(&query, tab);

    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_INT(tInt, found->type);
    TEST_ASSERT_EQUAL_INT(global_var, found->symbolType);
}

void test_analyze_ast_valid_assignment(void) {
    openLevel(tab);
    
    Symbol *sym_decl = makeSymbol("y", variable);
    sym_decl->type = tInt;
    addSymbolToTab(sym_decl, tab);

    Symbol *sym_y = makeSymbol("y", variable);
    ASTNode *node_y = makeLeaf(sym_y);

    Symbol *sym_5 = makeSymbol("5", intLiteral);
    sym_5->type = tInt;
    ASTNode *node_5 = makeLeaf(sym_5);

    Symbol *sym_assign = makeSymbol("=", assignOp);
    ASTNode *node_assign = makeNode(sym_assign, node_y, node_5);

    analyze_ast(node_assign, tab);
    TEST_ASSERT_TRUE(1); 
}

void test_analyze_ast_function_declaration(void) {
    openLevel(tab);

    Symbol *sym_1 = makeSymbol("1", intLiteral);
    sym_1->type = tInt;
    ASTNode *node_1 = makeLeaf(sym_1);

    Symbol *sym_ret = makeSymbol("return", returnNode);
    ASTNode *node_ret = makeNode(sym_ret, node_1, NULL);

    Symbol *sym_block = makeSymbol("block", block);
    ASTNode *node_block = makeNode(sym_block, node_ret, NULL);

    Symbol *sym_func = makeSymbol("sum", function);
    sym_func->type = tInt;
    ASTNode *node_func = makeNode(sym_func, NULL, node_block); 

    analyze_ast(node_func, tab);
    
    Symbol query = {.id = "sum"};
    Symbol *found = findSymbol(&query, tab);

    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_INT(tInt, found->type);
    TEST_ASSERT_EQUAL_INT(function, found->symbolType);
    TEST_ASSERT_EQUAL_INT(0, found->value.intValue);
}

void test_analyze_ast_if_statement(void) {
    openLevel(tab);
    
    Symbol *sym_true = makeSymbol("true", boolLiteral);
    sym_true->type = tBoolean;
    ASTNode *cond_node = makeLeaf(sym_true);
    
    Symbol *sym_block = makeSymbol("block", block);
    ASTNode *block_node = makeNode(sym_block, NULL, NULL);
    
    Symbol *sym_if = makeSymbol("if", ifNode);
    ASTNode *if_node = makeNode(sym_if, cond_node, block_node);
    
    analyze_ast(if_node, tab); 
    TEST_ASSERT_TRUE(1);
}

void test_analyze_ast_else_node(void) {
    openLevel(tab);
    
    Symbol *sym_block_if = makeSymbol("block", block);
    ASTNode *block_if = makeNode(sym_block_if, NULL, NULL);

    Symbol *sym_block_else = makeSymbol("block", block);
    ASTNode *block_else = makeNode(sym_block_else, NULL, NULL);

    Symbol *sym_else = makeSymbol("else", elseNode);
    ASTNode *else_node = makeNode(sym_else, block_if, block_else);

    analyze_ast(else_node, tab);
    TEST_ASSERT_TRUE(1);
}

void test_analyze_ast_while_and_negation(void) {
    openLevel(tab);
    
    Symbol *sym_false = makeSymbol("false", boolLiteral);
    sym_false->type = tBoolean;
    ASTNode *false_node = makeLeaf(sym_false);
    
    Symbol *sym_not = makeSymbol("!", negationOp);
    ASTNode *cond_node = makeNode(sym_not, false_node, NULL);
    
    Symbol *sym_block = makeSymbol("block", block);
    ASTNode *block_node = makeNode(sym_block, NULL, NULL);
    
    Symbol *sym_while = makeSymbol("while", whileNode);
    ASTNode *while_node = makeNode(sym_while, cond_node, block_node);
    
    analyze_ast(while_node, tab);
    TEST_ASSERT_TRUE(1);
}

void test_analyze_expression_complex_logical(void) {
    Symbol *sym_10 = makeSymbol("10", intLiteral);
    sym_10->type = tInt;
    ASTNode *node_10 = makeLeaf(sym_10);
    
    Symbol *sym_20 = makeSymbol("20", intLiteral);
    sym_20->type = tInt;
    ASTNode *node_20 = makeLeaf(sym_20);
    
    Symbol *sym_less = makeSymbol("<", lessOp);
    ASTNode *node_less = makeNode(sym_less, node_10, node_20);
    
    Symbol *sym_false = makeSymbol("false", boolLiteral);
    sym_false->type = tBoolean;
    ASTNode *node_false = makeLeaf(sym_false);
    
    Symbol *sym_or = makeSymbol("||", orOp);
    ASTNode *node_or = makeNode(sym_or, node_less, node_false);
    
    enum types res = analyze_expression(node_or, tab);
    TEST_ASSERT_EQUAL_INT(tBoolean, res);
}

void test_analyze_expression_unary_minus(void) {

    Symbol *sym_5 = makeSymbol("5", intLiteral);
    sym_5->type = tInt;
    ASTNode *node_5 = makeLeaf(sym_5);
    
    Symbol *sym_minus = makeSymbol("-", substractOp);
    ASTNode *node_minus = makeNode(sym_minus, node_5, NULL); 
    
    enum types res = analyze_expression(node_minus, tab);
    TEST_ASSERT_EQUAL_INT(tInt, res);
}

void test_analyze_expression_modulo(void) {
    Symbol *sym_10 = makeSymbol("10", intLiteral);
    sym_10->type = tInt;
    ASTNode *node_10 = makeLeaf(sym_10);
    
    Symbol *sym_3 = makeSymbol("3", intLiteral);
    sym_3->type = tInt;
    ASTNode *node_3 = makeLeaf(sym_3);
    
    Symbol *sym_mod = makeSymbol("%", modOp);
    ASTNode *node_mod = makeNode(sym_mod, node_10, node_3);
    
    enum types res = analyze_expression(node_mod, tab);
    TEST_ASSERT_EQUAL_INT(tInt, res);
}

#endif