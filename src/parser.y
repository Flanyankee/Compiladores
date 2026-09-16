%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    extern int yylex();
    void yyerror(const char *s);
%}

%token WHILE IF ELSE RETURN
%token <str> TYPE VOID BOOL DIGIT ALPHA
%token AND_OP OR_OP ADD_OP MULT_OP SUBTRACT_OP ASIGN_OP DIV_OP MOD_OP LESS_OP GREATER_OP EQUALS_OP 
%token DOT COMMA SEMICOLON LEFT_PARENTHESIS RIGHT_PARENTHESIS LEFT_BRACE RIGHT_BRACE UNDERSCORE EXCLAMATION NEW_LINE TAB SPACE

%left AND_OP OR_OP
%left ADD_OP SUBTRACT_OP MULT_OP DIV_OP MOD_OP

%%

program : v_d m_d
        ;

var_decl : TYPE id id_prime
         | SEMICOLON
         ;

v_d : COMMA v_d  var_decl 
    | 
    ;

id  : ALPHA alpha_num_prime
    | 
    ;

id_prime : id_prime id
         |
         ;


method_decl : t_v  id LEFT_PARENTHESIS TYPE id t_i RIGHT_PARENTHESIS block
            ;

m_d : m_d method_decl
    |
    ;

t_v : TYPE
    | VOID
    ;

t_i : COMMA t_i TYPE id 
    |
    ;

method_call : id LEFT_PARENTHESIS m_c RIGHT_PARENTHESIS

m_c : expr m_c_prime 
  |
  ;

m_c_prime : COMMA expr m_c_prime
        |
        ;

block : LEFT_BRACE v_d stat RIGHT_BRACE
      ;

statement : id ASIGN_OP expr SEMICOLON
    | method_decl SEMICOLON
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

stat : stat statement
     |
     ;

expr : id
    | method_call
    | literal
    | expr bin_op expr
    | SUBTRACT_OP expr
    | EXCLAMATION expr
    | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
    ;

bin_op  : arith_op
        | rel_op
        | cond_op
        ;

arith_op :  ADD_OP
    | SUBTRACT_OP
    | MULT_OP
    | DIV_OP
    | MOD_OP
    ;

rel_op  : LESS_OP
        | GREATER_OP
        | EQUALS_OP
        ;

cond_op : AND_OP
        | OR_OP
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