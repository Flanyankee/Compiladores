#ifndef SYMBOLS_TAB
#define SYMBOLS_TAB
#include "syntax-tree.h"

typedef struct SymbolTabElem {
  Symbol *symbol;
  struct SymbolTabElem *next;
  struct SymbolTabElem *back;
} SymbolTabElem;

typedef struct SymbolTab {
  struct SymbolTabElem *first;
  struct SymbolTabElem *last;
} SymbolTab;

SymbolTab *initializeSymbolTab();
void addSymbolToTab(Symbol *symbol, SymbolTab *tab);
Symbol *findSymbol(Symbol *symbol, SymbolTab *tab);
#endif
