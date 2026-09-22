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
%token DOT COMMA SEMICOLON LEFT_PARENTHESIS RIGHT_PARENTHESIS LEFT_BRACE RIGHT_BRACE UNDERSCORE EXCLAMATION NEW_LINE TAB SPACE COMENT

%left AND_OP OR_OP
%left ADD_OP SUBTRACT_OP MULT_OP DIV_OP MOD_OP
%left EQUALS_OP LESS_OP GREATER_OP
%right EXCLAMATION

%%

program : TYPE id decl_tail
        | VOID id method_decl
        ;

decl_tail   : id_prime SEMICOLON  
            | LEFT_PARENTHESIS TYPE id t_i RIGHT_PARENTHESIS block 
            ;

method_decl : LEFT_PARENTHESIS TYPE id t_i RIGHT_PARENTHESIS block
            ;

var_decl : TYPE id id_prime SEMICOLON
         ;

v_d : var_decl v_d
    | 
    ;

id  : ALPHA alpha_num_prime
    ;

id_prime : COMMA id id_prime
         |
         ;

t_i : COMMA TYPE id t_i
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

stat : statement stat
     |
     ;

expr : id
     | method_call
     | literal
     | expr ADD_OP expr
     | expr SUBTRACT_OP expr
     | expr MULT_OP expr
     | expr DIV_OP expr
     | expr MOD_OP expr
     | expr LESS_OP expr
     | expr GREATER_OP expr
     | expr EQUALS_OP expr
     | expr AND_OP expr
     | expr OR_OP expr
     | SUBTRACT_OP expr %prec MULT_OP
     | EXCLAMATION expr
     | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
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

