%top{
#include <stdio.h>
#include <stdlib.h>
#include "parser.tab.h"
extern FILE *outputFilePtr;
}

%option noyywrap
%option yylineno

%x COMENTARIO

TYPE ("int"|"boolean"|"float")
VOID "void"
INT [0-9]+
FLOAT [0-9]+"."[0-9]+
BOOL ("true"|"false")
IF "if"
ELSE "else"
WHILE "while"
RETURN "return"
ID [a-zA-Z][a-zA-Z0-9_]*

%%

{TYPE} {yylval.str = strdup(yytext); return TYPE;}
{VOID} {return VOID;}
{IF} {return IF;}
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{RETURN} {return RETURN;}
{BOOL} {yylval.str = strdup(yytext); return BOOL;}
{FLOAT} {yylval.str = strdup(yytext); return FLOAT;}
{INT} {yylval.str = strdup(yytext); return INT;}

"&&" {return AND_OP;}
"||" {return OR_OP;}
"!" {return EXCLAMATION;}
"+" {return ADD_OP;}
"-" {return SUBTRACT_OP;}
"*" {return MULT_OP;}
"/" {return DIV_OP;}
"%" {return MOD_OP;}
"==" {return EQUALS_OP;}
"=" {return ASIGN_OP;}
"<" {return LESS_OP;}
">" {return GREATER_OP;}
"." {return DOT;}
"," {return COMMA;}
";" {return SEMICOLON;}
"(" {return LEFT_PARENTHESIS;}
")" {return RIGHT_PARENTHESIS;}
"{" {return LEFT_BRACE;}
"}" {return RIGHT_BRACE;}
"_" {return UNDERSCORE;}
"\n" { }
"\t" { }
" " { }

"//".* { }

"/*"               { BEGIN(COMENTARIO);  }
<COMENTARIO>"*/"   { BEGIN(INITIAL);     }
<COMENTARIO>\n     { }
<COMENTARIO>.      { }
<COMENTARIO><<EOF>> { 
		     if (outputFilePtr) {
                     	fprintf(outputFilePtr, "Error lexico: Comentario sin cerrar en la linea %d\n", yylineno); 
			exit(1);
		     }
                     BEGIN(INITIAL); 
                   }

{ID} {yylval.str = strdup(yytext); return ID;}

. {if (outputFilePtr) { 
	fprintf(outputFilePtr, "Error lexico: Caracter %s no reconocido en la linea %d\n", yytext, yylineno);
	exit(1);
   }} 
