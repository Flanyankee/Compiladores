#include "symbols-tab.h"
#include <stdlib.h>
#include <string.h>

SymbolTab *initializeSymbolTab(){
    SymbolTab *tab = (SymbolTab *) malloc(sizeof(SymbolTab));
    tab->firstLevel = (SymbolTabLevel *) malloc(sizeof(SymbolTabLevel));
    tab->lastLevel = (SymbolTabLevel *) malloc(sizeof(SymbolTabLevel));
	
    (tab->firstLevel)->up = tab->lastLevel;
	(tab->firstLevel)->down = NULL;
	(tab->lastLevel)->down = tab->firstLevel;
	(tab->lastLevel)->up = NULL;
	(tab->firstLevel)->first = NULL;
	(tab->firstLevel)->last = NULL;
	(tab->lastLevel)->first = NULL;
	(tab->lastLevel)->last = NULL;

	return tab;
}

void addSymbolToTab(Symbol *symbol, SymbolTab *tab) {
	SymbolTabLevel *auxLevel = (tab->lastLevel)->down;
	SymbolTabElem *elem = (SymbolTabElem *)malloc(sizeof(SymbolTabElem));
	elem->symbol = symbol;
	elem->back = (auxLevel->last)->back;
	elem->next = auxLevel->last;
	(elem->back)->next = elem;
	(auxLevel->last)->back = elem;
}

Symbol *findSymbol(Symbol *symbol, SymbolTab *tab) {
	SymbolTabLevel *auxLevel = (tab->lastLevel)->down;
	SymbolTabElem *aux = auxLevel->first->next;
	while (aux->next != NULL) {
		if (strcmp(aux->symbol->id, symbol->id) == 0) {
			return aux->symbol;
		}
		aux = aux->next;
	}
	return NULL;
}

void openLevel(SymbolTab *tab){
    SymbolTabLevel *level = (SymbolTabLevel *) malloc(sizeof(SymbolTabLevel));
    level->first = (SymbolTabElem *) malloc(sizeof(SymbolTabElem));
    level->last = (SymbolTabElem *) malloc(sizeof(SymbolTabElem));
	level->up = (SymbolTabLevel *) malloc(sizeof(SymbolTabLevel));
	level->down = (SymbolTabLevel *) malloc(sizeof(SymbolTabLevel));

    (level->first)->next = level->last;
	(level->first)->back = NULL;
	(level->last)->back = level->first;
	(level->last)->next = NULL;

	level->up = tab->lastLevel;
	level->down = (tab->lastLevel)->down;
	(level->down)->up = level;
	(tab->lastLevel)->down = level;
}

void closeLevel(SymbolTab *tab){
	SymbolTabLevel *aux = (tab->lastLevel)->down;
	(tab->lastLevel)->down = (aux)->down;
	(aux->down)->up = tab->lastLevel;

	SymbolTabElem *aux2 = aux->first;
	while(aux2->next != NULL){
		aux2 = aux2->next;
		free(aux2->back);
	}

	free(aux2);
	free(aux);
}