%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();

void yyerror(const char *s);
%}

%token T_TYPE
%token T_RETURN
%token T_BOOL
%token T_NUM
%token T_ID
%token OP_AND
%token OP_OR
%token OP_SUMA
%token OP_MULT
%token OP_RESTA
%token OP_ASIGN
%token T_PUNTO_COMA
%token T_PAR_IZQ
%token T_PAR_DER
%token T_LLAVE_IZQ
%token T_LLAVE_DER

%left OP_AND OP_OR
%left OP_SUMA OP_RESTA OP_MULT

%%

S : T_TYPE  T_ID  T_PAR_IZQ T_PAR_DER T_LLAVE_IZQ P T_LLAVE_DER {printf("\nSe detecto el cuerpo de una funcion\n");}
  ;

P : E_triple E_prima
  ;

R : T_RETURN T_PUNTO_COMA
  | T_RETURN E T_PUNTO_COMA
  ;

E_triple : T_TYPE T_ID T_PUNTO_COMA E_triple
	 | T_TYPE T_ID OP_ASIGN E T_PUNTO_COMA E_triple
	 | 
	 ;

E_doble : T_ID OP_ASIGN E T_PUNTO_COMA
	| R
	;

E_prima : E_doble E_prima
	| 
	;

E : E OP_SUMA E
  | E OP_MULT E
  | T_PAR_IZQ E T_PAR_DER
  | E OP_AND E
  | E OP_OR E
  | T_NUM
  | T_BOOL
  | T_ID
  ;

%%

void yyerror(const char *s) {
	fprintf(stderr, "\n error de sintaxis: %s\n", s);
	exit(1);
}

int main(){
	yyparse();
}
