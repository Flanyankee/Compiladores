%top{
	#include <stdio.h>
	#include "cli.tab.h"
}

CHAR ([a-z]|[A-Z])
NUM [0-9]
FILE_NAME ({CHAR}|{NUM})({CHAR}|{NUM}|"-")*

SOURCE_FILE {FILE_NAME}".ctds"
OUTPUT_FILE {FILE_NAME}
PREFIX "c-tds"
TARGET "-target"
TARGET_OPTIONS ("scan"|"parse"|"codinter"|"assembly")
OUTPUT "-o"
OPTIMIZE "-opt"
OPTIMIZE_ARGS "all"
DEBUG "-debug"
HELP ("-help"|"-h")

%%

{PREFIX} {yylval.str = strdup(yytext); return PREFIX;}
{SOURCE_FILE} {yylval.str = strdup(yytext); return SOURCE_FILE;}
{TARGET} {yylval.str = strdup(yytext); return TARGET;}
{TARGET_OPTIONS} {yylval.str = strdup(yytext); return TARGET_OPTIONS;}
{OPTIMIZE} {yylval.str = strdup(yytext); return OPTIMIZE;}
{OUTPUT} {yylval.str = strdup(yytext); return OUTPUT;}
{OPTIMIZE_ARGS} {yylval.str = strdup(yytext); return OPTIMIZE_ARGS;}
{DEBUG} {yylval.str = strdup(yytext); return DEBUG;}
{HELP} {yylval.str = strdup(yytext); return HELP;}
{CHAR} {yylval.str = strdup(yytext); return CHAR;}
{NUM} {yylval.str = strdup(yytext); return NUM;}
{OUTPUT_FILE} {yylval.str = strdup(yytext); return OUTPUT_FILE;}
"\n" {yylval.str = strdup(yytext); return NEW_LINE;}
" " { }

. { printf("Syntax error: %s\n", yytext); exit(2);}
