#include "symbols-tab.h"
#include <stdlib.h>
#include <string.h>

SymbolTab* initializeSymbolTab() {
	SymbolTab* tab = (SymbolTab*)malloc(sizeof(SymbolTab));
	tab->first = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	tab->last = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	(tab->first)->next = tab->last;
	(tab->first)->back = NULL;
	(tab->last)->back = tab->first;
	(tab->last)->next = NULL;
	return tab;
}
void addSymbolToTab(Symbol* symbol, SymbolTab* tab) {
	SymbolTabElem* elem = (SymbolTabElem*)malloc(sizeof(SymbolTabElem));
	elem->symbol = symbol;
	elem->back = (tab->last)->back;
	elem->next = tab->last;
	(elem->back)->next = elem;
	(tab->last)->back = elem;
}
Symbol* findSymbol(Symbol* symbol, SymbolTab* tab) {
	SymbolTabElem* aux = tab->first->next;
	while (aux->next != NULL) {
		if (strcmp(aux->symbol->id, symbol->id) == 0) {
			return aux->symbol;
		}
		aux = aux->next;
	}
	return NULL;
}
