%top{
#include <stdio.h>
enum Tokens {
    T_TYPE, T_RETURN, T_BOOL, T_NUM, T_ID, 
    T_AND, T_OR, T_SUMA, T_MULT, T_RESTA, 
    T_ASIGN, T_PUNTO_COMA, T_PAR_IZQ, T_PAR_DER, 
    T_LLAVE_IZQ, T_LLAVE_DER
};
}

TYPE ("int"|"bool"|"void")
NUM "-"?[0-9]+
BOOL ("true"|"false")
ID ([a-z]|[A-Z])([0-9]|[a-z]|[A-Z])*
RETURN "return"

%%

{TYPE} {return T_TYPE;}
{RETURN} {return T_RETURN;}
{BOOL} {return T_BOOL;}
{NUM} {return T_NUM;}
{ID} {return T_ID;}

"&&" {return T_AND;}
"||" {return T_OR;}
"+" {return T_SUMA;}
"*" {return T_MULT;}
"-" {return T_RESTA;}
"=" {return T_ASIGN;}
";" {return T_PUNTO_COMA;}
"(" {return T_PAR_IZQ;}
")" {return T_PAR_DER;}
"{" {return T_LLAVE_IZQ;}
"}" {return T_LLAVE_DER;}

. { }

%%
int main() {
	yylex();
}
