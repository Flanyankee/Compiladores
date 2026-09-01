%top{
#include <stdio.h>
#include "extended-language.tab.h"
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

"&&" {return OP_AND;}
"||" {return OP_OR;}
"+" {return OP_SUMA;}
"*" {return OP_MULT;}
"-" {return OP_RESTA;}
"=" {return OP_ASIGN;}
";" {return T_PUNTO_COMA;}
"(" {return T_PAR_IZQ;}
")" {return T_PAR_DER;}
"{" {return T_LLAVE_IZQ;}
"}" {return T_LLAVE_DER;}

. { }
