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

program : var_decl method_decl

var_decl : TYPE { id } COMMA
         | SEMICOLON

method_decl :  { TYPE | VOID } id ( [ {TYPE id} COMMA ] ) block

block : LEFT_BRACE var_decl statement RIGHT_BRACE

statement : id ASIGN_OP expr SEMICOLON
    | method_decl SEMICOLON
    | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block
    | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block ELSE block
    | WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block
    | RETURN expr SEMICOLON
    | SEMICOLON
    | block

method_call : id LEFT_PARENTHESIS expr RIGHT_PARENTHESIS

expr : id
    | method_call
    | literal
    | expr bin_op expr
    | SUBTRACT_OP expr
    | EXCLAMATION expr
    | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS
    
bin_op  : arith_op
        | rel_op
        | cond_op

arith_op :  ADD_OP
    | SUBTRACT_OP
    | MULT_OP
    | DIV_OP
    | MOD_OP

rel_op  : LESS_OP
        | GREATER_OP
        | EQUALS_OP

cond_op : AND_OP
        | OR_OP

literal : int_literal
        | bool_literal
        | float_literal

id  : ALPHA
    | alpha_num
    | UNDERSCORE

alpha_num   : ALPHA
            | DIGIT

int_literal : DIGIT

bool_literal : BOOL

float_literal: DIGIT DOT DIGIT

%%