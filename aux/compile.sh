flex --header-file=header/scanner.h -o src/lex.yy.c src/scanner.lex && 
bison src/parser.y -o src/parser.tab.c --defines=header/parser.tab.h &&
ceedling release
