%top{
#include <stdio.h>
#include "parser.tab.h"
}

%option noyywrap

TYPE ("int"|"boolean"|"float")
VOID "void"
DIGIT [0-9]
BOOL ("true"|"false")
ALPHA ([a-z]|[A-Z])
IF "if"
ELSE "else"
WHILE "while"
RETURN "return"

%%

{TYPE} {return TYPE;}
{VOID} {return VOID;}
{IF} {return IF;}
{ELSE} {return ELSE;}
{WHILE} {return WHILE;}
{RETURN} {return RETURN;}
{BOOL} {return BOOL;}
{DIGIT} {return DIGIT;}
{ALPHA} {return ALPHA;}

"&&" {return AND_OP;}
"||" {return OR_OP;}
"!" {return EXCLAMATION;}
"+" {return ADD_OP;}
"-" {return SUBTRACT_OP;}
"*" {return MULT_OP;}
"/" {return DIV_OP;}
"%" {return MOD_OP;}
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
"\n" {return NEW_LINE;}
"\t" {return TAB;}
" " {return SPACE;}
"//" {return COMENT;}


. { }
