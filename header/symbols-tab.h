#ifndef SYMBOLS_TAB
#define SYMBOLS_TAB
#include "syntax-tree.h"

typedef struct SymbolTabElem {
	Symbol *symbol;
	struct SymbolTabElem *next;
	struct SymbolTabElem *back;
} SymbolTabElem;

typedef struct SymbolTabLevel {
	struct SymbolTabElem *first;
	struct SymbolTabElem *last;
	struct SymbolTabLevel *up;
	struct SymbolTabLevel *down;
} SymbolTabLevel;

typedef struct SymbolTab {
	struct SymbolTabLevel *firstLevel;
	struct SymbolTabLevel *lastLevel;
} SymbolTab;

SymbolTab *initializeSymbolTab();

void addSymbolToTab(Symbol *symbol, SymbolTab *tab);
Symbol *findSymbol(Symbol *symbol, SymbolTab *tab);

void openLevel(SymbolTab *tab);
void closeLevel(SymbolTab *tab);
#endif
