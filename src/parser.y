%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "scanner.h"

    void yyerror(const char *s);
%}

%union {
	char* str;
}


%token WHILE IF ELSE RETURN
%token <str> TYPE VOID BOOL DIGIT ALPHA
%token AND_OP OR_OP ADD_OP MULT_OP SUBTRACT_OP ASIGN_OP DIV_OP MOD_OP LESS_OP GREATER_OP EQUALS_OP 
%token DOT COMMA SEMICOLON LEFT_PARENTHESIS RIGHT_PARENTHESIS LEFT_BRACE RIGHT_BRACE UNDERSCORE EXCLAMATION 

%left AND_OP OR_OP
%left ADD_OP SUBTRACT_OP MULT_OP DIV_OP MOD_OP
%right EXCLAMATION 
%nonassoc GREATER_OP LESS_OP EQUALS_OP

%%

program : v_d m_d
        ;

v_d : v_d var_decl //v_d es var declaration
    | 
    ;

var_decl : TYPE id id_prime SEMICOLON
         ;

id_prime : COMMA id id_prime
         |
         ;

id  : ALPHA alpha_num_prime
    ;

m_d : method_decl m_d //m_d es method declaration
    |
    ;

method_decl : TYPE id LEFT_PARENTHESIS t_i RIGHT_PARENTHESIS block
	        | VOID id LEFT_PARENTHESIS t_i RIGHT_PARENTHESIS block
            ;

t_i : TYPE id t_i_2 //t_i es type id
    | 
    ;

t_i_2 : COMMA TYPE id t_i_2
      | 
      ;

method_call : id LEFT_PARENTHESIS m_c RIGHT_PARENTHESIS

m_c : expr m_c_prime //m_c es method call
  |
  ;

m_c_prime : COMMA expr m_c_prime
        |
        ;

block : LEFT_BRACE v_d stat RIGHT_BRACE
      ;

statement : id ASIGN_OP expr SEMICOLON
    | method_call SEMICOLON
    | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block else
    | WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block
    | RETURN SEMICOLON 
    | RETURN expr SEMICOLON
    | SEMICOLON
    | block
    ;

else : ELSE block
     |
     ;

stat : statement stat //stat es statement
     |
     ;

expr : id
     | method_call
     | literal
     | bin_op
     | SUBTRACT_OP expr
     | EXCLAMATION expr
     | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
     ;

bin_op : arith_op
       | rel_op
       | cond_op
       ;

arith_op :  expr ADD_OP expr 
    | expr SUBTRACT_OP expr
    | expr MULT_OP expr
    | expr DIV_OP expr
    | expr MOD_OP expr
    ;

rel_op  : expr LESS_OP expr
        | expr GREATER_OP expr
        | expr EQUALS_OP expr
        ;

cond_op : expr AND_OP expr
        | expr OR_OP expr
        ;

literal : int_literal
        | bool_literal
        | float_literal
        ;

alpha_num   : ALPHA
            | DIGIT
            | UNDERSCORE
            ;

alpha_num_prime : alpha_num alpha_num_prime
                | 
                ;

int_literal : DIGIT int_literal_prime
            ;

int_literal_prime : int_literal
                  | 
                  ;

bool_literal: BOOL
            ;

float_literal: int_literal DOT int_literal
             ;

%%

void yyerror(const char* s) {
        fprintf(stderr, "Error Sintactico: %s en la linea %d\n", yytext, yylineno);
}