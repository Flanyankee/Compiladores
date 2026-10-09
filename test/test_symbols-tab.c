#include "unity.h"
#include "symbols-tab.h"
#include <stdlib.h>
#include <string.h>

static SymbolTab *tab = NULL;


static Symbol* create_test_symbol(const char *id) {
    Symbol *sym = (Symbol *)malloc(sizeof(Symbol));
    sym->id = (char *)malloc(strlen(id) + 1);
    strcpy(sym->id, id);
    return sym;
}

static void free_test_symbol(Symbol *symbol) {
    if (symbol != NULL) {
        if (symbol->id != NULL) {
            free(symbol->id);
        }
        free(symbol);
    }
}

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

void test_initializeSymbolTab_creates_limit_nodes(void) {
    TEST_ASSERT_NOT_NULL(tab);
    TEST_ASSERT_NOT_NULL(tab->firstLevel);
    TEST_ASSERT_NOT_NULL(tab->lastLevel);

    TEST_ASSERT_EQUAL_PTR(tab->lastLevel, tab->firstLevel->up);
    TEST_ASSERT_NULL(tab->firstLevel->down);
    TEST_ASSERT_EQUAL_PTR(tab->firstLevel, tab->lastLevel->down);
    TEST_ASSERT_NULL(tab->lastLevel->up);

    TEST_ASSERT_NULL(tab->firstLevel->first);
    TEST_ASSERT_NULL(tab->firstLevel->last);
    TEST_ASSERT_NULL(tab->lastLevel->first);
    TEST_ASSERT_NULL(tab->lastLevel->last);
}

void test_openLevel_configures_scope(void) {
    openLevel(tab);
    SymbolTabLevel *current = (tab->lastLevel)->down;

    TEST_ASSERT_NOT_NULL(current);
    TEST_ASSERT_NOT_EQUAL(tab->firstLevel, current);
    TEST_ASSERT_EQUAL_PTR(tab->lastLevel, current->up);
    TEST_ASSERT_EQUAL_PTR(tab->firstLevel, current->down);
    TEST_ASSERT_EQUAL_PTR(current, tab->firstLevel->up);

    TEST_ASSERT_NOT_NULL(current->first);
    TEST_ASSERT_NOT_NULL(current->last);
    TEST_ASSERT_EQUAL_PTR(current->last, (current->first)->next);
    TEST_ASSERT_EQUAL_PTR(current->first, (current->last)->back);
    TEST_ASSERT_NULL((current->first)->back);
    TEST_ASSERT_NULL((current->last)->next);
}

void test_addSymbolToTab_and_findSymbol_existing(void) {
    openLevel(tab);

    Symbol *sym = create_test_symbol("variable_x");
    addSymbolToTab(sym, tab);

    Symbol query;
    query.id = "variable_x";

    Symbol *found = findSymbol(&query, tab);
    TEST_ASSERT_NOT_NULL(found);
    TEST_ASSERT_EQUAL_PTR(sym, found);
    TEST_ASSERT_EQUAL_STRING("variable_x", found->id);

    free_test_symbol(sym);
}

void test_findSymbol_returns_null_when_not_found(void) {
    openLevel(tab);

    Symbol *sym = create_test_symbol("var_declared");
    addSymbolToTab(sym, tab);

    Symbol query;
    query.id = "var_no_declared";

    Symbol *found = findSymbol(&query, tab);
    TEST_ASSERT_NULL(found);

    free_test_symbol(sym);
}

void test_add_multiple_symbols_in_current_scope(void) {
    openLevel(tab);

    Symbol *symbol1 = create_test_symbol("alpha");
    Symbol *symbol2 = create_test_symbol("beta");
    Symbol *symbol3 = create_test_symbol("gamma");

    addSymbolToTab(symbol1, tab);
    addSymbolToTab(symbol2, tab);
    addSymbolToTab(symbol3, tab);

    Symbol query1 = {.id = "alpha"};
    Symbol query2 = {.id = "beta"};
    Symbol query3 = {.id = "gamma"};

    TEST_ASSERT_EQUAL_PTR(symbol1, findSymbol(&query1, tab));
    TEST_ASSERT_EQUAL_PTR(symbol2, findSymbol(&query2, tab));
    TEST_ASSERT_EQUAL_PTR(symbol3, findSymbol(&query3, tab));

    free_test_symbol(symbol1);
    free_test_symbol(symbol2);
    free_test_symbol(symbol3);
}

void test_closeLevel_restores_previous_scope(void) {
    openLevel(tab); 
    SymbolTabLevel *level1 = (tab->lastLevel)->down;

    openLevel(tab); 
    SymbolTabLevel *level2 = (tab->lastLevel)->down;
    TEST_ASSERT_NOT_EQUAL(level1, level2);

    closeLevel(tab);

    TEST_ASSERT_EQUAL_PTR(level1, (tab->lastLevel)->down);
    TEST_ASSERT_EQUAL_PTR(tab->lastLevel, level1->up);
}

void test_closeLevel_destroys_local_symbols(void) {
    openLevel(tab);
    Symbol *s_global = create_test_symbol("global_var");
    addSymbolToTab(s_global, tab);

    openLevel(tab);
    Symbol *s_local = create_test_symbol("local_var");
    addSymbolToTab(s_local, tab);

    Symbol q_local = {.id = "local_var"};
    TEST_ASSERT_NOT_NULL(findSymbol(&q_local, tab));

    closeLevel(tab);

    TEST_ASSERT_NULL(findSymbol(&q_local, tab));

    free_test_symbol(s_global);
    free_test_symbol(s_local);
}

void test_findSymbolMultiLevel(void){
    SymbolTab* auxTab = initializeSymbolTab();
    openLevel(auxTab);

    Symbol* simbolY = create_test_symbol("Y");
    addSymbolToTab(simbolY, auxTab);

    openLevel(auxTab);

    Symbol* simbolX = create_test_symbol("X");
    addSymbolToTab(simbolX, auxTab);

    findSymbolMultiLevel(simbolY, auxTab);

    TEST_ASSERT_NOT_NULL(findSymbol(simbolX, auxTab));

    closeLevel(auxTab);
    closeLevel(auxTab);
    free_test_symbol(simbolX);
    free_test_symbol(simbolY);
}

void test_findSymbolMultiLevel_v2(void){
    SymbolTab* auxTab = initializeSymbolTab();
    openLevel(auxTab);

    Symbol* simbolY = create_test_symbol("Y");
    addSymbolToTab(simbolY, auxTab);

    openLevel(auxTab);

    Symbol* simbolX = create_test_symbol("X");
    addSymbolToTab(simbolX, auxTab);

    SymbolTabElem* lastLevel = auxTab->lastLevel->first;

    findSymbolMultiLevel(simbolY, auxTab);

    TEST_ASSERT_EQUAL(auxTab->lastLevel->first, lastLevel);

    closeLevel(auxTab);
    closeLevel(auxTab);
    free_test_symbol(simbolX);
    free_test_symbol(simbolY);
}