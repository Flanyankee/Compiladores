%{
#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
void yyerror(const char* s);
%}

%token <str> CHAR NUM
%token <str> PREFIX TARGET TARGET_OPTIONS OUTPUT OUTPUT_FILE OPTIMIZE OPTIMIZE_ARGS DEBUG HELP NEW_LINE SOURCE_FILE

%union {char* str;}

%%

CLI : PREFIX Target Output Optimizations Debug SOURCE_FILE NEW_LINE {prefix = $1; YYACCEPT;}
    | PREFIX HELP NEW_LINE {prefix = $1; helpFlag = $2; YYACCEPT;}
    | PREFIX NEW_LINE {prefix = $1; helpFlag = "-h"; YYACCEPT;}

Target : TARGET TARGET_OPTIONS {targetStage = $2;}
       | //lambda

Output : OUTPUT OUTPUT_FILE {outputFile = $2;}
       | OUTPUT CHAR {outputFile = $2;}
       | OUTPUT NUM {outputFile = $2;}
       | //lambda

Optimizations : OPTIMIZE OPTIMIZE_ARGS {optimization = $2;}
	      | //lambda

Debug : DEBUG {debugFlag = $1;}
      | // lambda

%%

void yyerror(const char* s) {
	fprintf(stderr, "Error: %s\n", s);
}
