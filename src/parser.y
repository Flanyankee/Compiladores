%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "scanner.h"
    #include "syntax-tree.h"
    #include "constants.h"

    extern FILE *outputFilePtr;
    void yyerror(const char *s);

    extern ASTNode* syntaxTree;
    extern char* declType;
%}

%union {
	char* str;
	struct ASTNode* node;
}


%token WHILE IF ELSE RETURN
%token <str> TYPE VOID BOOL INT FLOAT ID
%token AND_OP OR_OP ADD_OP MULT_OP SUBTRACT_OP ASIGN_OP DIV_OP MOD_OP LESS_OP GREATER_OP EQUALS_OP 
%token DOT COMMA SEMICOLON LEFT_PARENTHESIS RIGHT_PARENTHESIS LEFT_BRACE RIGHT_BRACE UNDERSCORE EXCLAMATION 

%left AND_OP OR_OP
%left ADD_OP SUBTRACT_OP MULT_OP DIV_OP MOD_OP
%right EXCLAMATION 
%nonassoc GREATER_OP LESS_OP EQUALS_OP

%type <node> program v_d var_decl id_prime m_d method_decl t_i t_i_2 method_call m_c m_c_prime block expr stat statement else bin_op arith_op rel_op cond_op literal

%%

program : v_d m_d {
		Symbol* programSymbol = makeSymbol("", program);
		syntaxTree = makeNode(programSymbol, $1, $2);
		$$ = syntaxTree;
		}
        ;

v_d : v_d var_decl {
		Symbol* symbol = makeSymbol("", varDeclarationList);
		ASTNode* vd = makeNode(symbol, $2, $1);
		$$ = vd;
    		}
    | { $$ = NULL; }
    ;

var_decl : TYPE ID {declType = $1;} id_prime SEMICOLON {
		Symbol* varDeclSym = makeSymbol("", varDeclaration);
		Symbol* idPrime = makeSymbol("", idList);
		Symbol* id = makeSymbol($2, variable);

		if (strcmp($1, "int") == 0) {
			varDeclSym->type = tInt;
		} else if (strcmp($1, "boolean") == 0) {
			varDeclSym->type = tBoolean;
		} else {
			varDeclSym->type = tFloat;
		}
		
		idPrime->type = varDeclSym->type;
		id->type = varDeclSym->type;

		ASTNode* idTreeNode = makeLeaf(id);
		ASTNode* idListNode = makeNode(idPrime, idTreeNode, $4);
		ASTNode* varDeclNode = makeNode(varDeclSym, idListNode, NULL);
		$$ = varDeclNode;
		}
         ;

id_prime : COMMA ID id_prime { 
		Symbol* idPrime = makeSymbol("", idList);
		Symbol* id = makeSymbol($2, variable);

		if (strcmp(declType, "int") == 0) {
			id->type = tInt;
		} else if (strcmp(declType, "boolean") == 0) {
			id->type = tBoolean;
		} else {
			id->type = tFloat;
		}

		ASTNode* idTreeNode = makeLeaf(id);
		ASTNode* idListNode = makeNode(idPrime, idTreeNode, $3);
		$$ = idListNode;
	 }
         | {$$ = NULL;}
         ;

m_d : method_decl m_d {
		Symbol* symbol = makeSymbol("", methodDeclarationList);
		ASTNode* md = makeNode(symbol, $1, $2);
		$$ = md;
    }
    | {$$ = NULL;}
    ;

method_decl : TYPE ID LEFT_PARENTHESIS t_i RIGHT_PARENTHESIS block {
		Symbol* funcSym = makeSymbol($2, function);

		if (strcmp($1, "int") == 0) {
			funcSym->type = tInt;
		} else if (strcmp($1, "boolean") == 0) {
			funcSym->type = tBoolean;
		} else {
			funcSym->type = tFloat;
		}

		ASTNode* funcNode = makeNode(funcSym, $4, $6);
		$$ = funcNode;
		}
	    | VOID ID LEFT_PARENTHESIS t_i RIGHT_PARENTHESIS block {
		Symbol* funcSym = makeSymbol($2, function);
		funcSym->type = tVoid;
		ASTNode* funcNode = makeNode(funcSym, $4, $6);
		$$ = funcNode;
		}
            ;

t_i : TYPE ID t_i_2 {
	Symbol* parameterSym = makeSymbol($2, formalParameter);
	Symbol* parameterListSym = makeSymbol("", formalParameterList);

	if (strcmp($1, "int") == 0) {
		parameterSym->type = tInt;
	} else if (strcmp($1, "boolean") == 0) {
		parameterSym->type = tBoolean;
	} else {
		parameterSym->type = tFloat;
	}
	parameterListSym->type = parameterSym->type;

	ASTNode* formalParameterNode = makeLeaf(parameterSym);
	ASTNode* parameterListNode = makeNode(parameterListSym, formalParameterNode, $3);
	$$ = parameterListNode;
	}
    | {$$ = NULL;}
    ;

t_i_2 : COMMA TYPE ID t_i_2 {
	Symbol* parameterSym = makeSymbol($3, formalParameter);
	Symbol* parameterListSym = makeSymbol("", formalParameterList);

	if (strcmp($2, "int") == 0) {
		parameterSym->type = tInt;
	} else if (strcmp($2, "boolean") == 0) {
		parameterSym->type = tBoolean;
	} else {
		parameterSym->type = tFloat;
	}
	parameterListSym->type = parameterSym->type;

	ASTNode* formalParameterNode = makeLeaf(parameterSym);
	ASTNode* parameterListNode = makeNode(parameterListSym, formalParameterNode, $4);
	}
      | {$$ = NULL;}
      ;

method_call : ID LEFT_PARENTHESIS m_c RIGHT_PARENTHESIS {
	    Symbol* funcInvokeSym = makeSymbol($1, functionInvoke);
	    ASTNode* funcInvokeNode = makeNode(funcInvokeSym, $3, NULL);
	    $$ = funcInvokeNode;
	    }
	    ;

m_c : expr m_c_prime /* m_c son los parametros actuales */ {
    Symbol* parameterListSym = makeSymbol("", actualParameterList);
    ASTNode* parameterListNode = makeNode(parameterListSym, $1, $2);
    $$ = parameterListNode;
    }
    | {$$ = NULL;}
    ;

m_c_prime : COMMA expr m_c_prime {
	  Symbol* parameterListSym = makeSymbol("", actualParameterList);
	  ASTNode* parameterListNode = makeNode(parameterListSym, $2, $3);
	  $$ = parameterListNode;
	  }
	  | {$$ = NULL;}
	  ;

block : LEFT_BRACE v_d stat RIGHT_BRACE {
      Symbol* blockSymbol = makeSymbol("", block);
      ASTNode* blockTreeNode = makeNode(blockSymbol, $2, $3);
      $$ = blockTreeNode;
      }
      ;

statement : ID ASIGN_OP expr SEMICOLON {
		Symbol* assignSymbol = makeSymbol("", assignOp);
		Symbol* idSymbol = makeSymbol($1, variable);
		ASTNode* idTreeNode = makeLeaf(idSymbol);
		ASTNode* assignNode = makeNode(assignSymbol, idTreeNode, $3);
		$$ = assignNode;
		}
	  | method_call SEMICOLON { $$ = $1; }
	  | IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block else {
		Symbol* ifSymbol = makeSymbol("", ifNode);
		Symbol* ifBodySymbol = makeSymbol("", blankNode);
		ASTNode* ifBodyNode = makeNode(ifBodySymbol, $5, $6);
		ASTNode* ifTreeNode = makeNode(ifSymbol, $3, ifBodyNode);
		$$ = ifTreeNode;
		}
	  | WHILE LEFT_PARENTHESIS expr RIGHT_PARENTHESIS block {
		Symbol* whileSymbol = makeSymbol("", whileNode);
		ASTNode* whileTreeNode = makeNode(whileSymbol, $3, $5);
		$$ = whileTreeNode;
		}
	  | RETURN SEMICOLON {
		Symbol* returnSymbol = makeSymbol("return", returnNode);
		returnSymbol->type = tVoid;
		ASTNode* returnTreeNode = makeLeaf(returnSymbol);
		$$ = returnTreeNode;
		}
	  | RETURN expr SEMICOLON {
		Symbol* returnSymbol = makeSymbol("return", returnNode);
		ASTNode* returnTreeNode = makeNode(returnSymbol, $2, NULL);
		$$ = returnTreeNode;
		}
	  | SEMICOLON { $$ = NULL; }
	  | block { $$ = $1; }
	  ;

else : ELSE block {
	Symbol* elseSymbol = makeSymbol("", elseNode);
	ASTNode* elseTreeNode = makeNode(elseSymbol, $2, NULL);
	$$ = elseTreeNode;
     }
     | {$$ = NULL;}
     ;

stat : statement stat {
     Symbol* statementListSym = makeSymbol("", statementList);
     ASTNode* statementNode = makeNode(statementListSym, $1, $2);
     $$ = statementNode;
     }
     | {$$ = NULL;}
     ;

expr : ID {
	Symbol* idSymbol = makeSymbol($1, variable);
	ASTNode* exprNode = makeLeaf(idSymbol);
	$$ = exprNode;
     }
     | method_call { $$ = $1; }
     | literal { $$ = $1; }
     | bin_op { $$ = $1; }
     | SUBTRACT_OP expr {
	Symbol* substractSymbol = makeSymbol("", substractOp);
	ASTNode* substractNode = makeNode(substractSymbol, $2, NULL);
	$$ = substractNode;
     }
     | EXCLAMATION expr {
	Symbol* negationSymbol = makeSymbol("", negationOp);
	ASTNode* negationNode = makeNode(negationSymbol, $2, NULL);
	$$ = negationNode;
     }
     | LEFT_PARENTHESIS expr RIGHT_PARENTHESIS { $$ = $2; }
     ;

bin_op : arith_op { $$ = $1; }
       | rel_op { $$ = $1; }
       | cond_op { $$ = $1; }
       ;

arith_op : expr ADD_OP expr {
		Symbol* addSymbol = makeSymbol("", addOp);
		ASTNode* addNode = makeNode(addSymbol, $1, $3);
		$$ = addNode;
	 }
	 | expr SUBTRACT_OP expr {
		Symbol* substractSymbol = makeSymbol("", substractOp);
		ASTNode* substractNode = makeNode(substractSymbol, $1, $3);
		$$ = substractNode;
	 }
	 | expr MULT_OP expr {
		Symbol* multSymbol = makeSymbol("", multOp);
		ASTNode* multNode = makeNode(multSymbol, $1, $3);
		$$ = multNode;
	 }
	 | expr DIV_OP expr {
		Symbol* divSymbol = makeSymbol("", divOp);
		ASTNode* divNode = makeNode(divSymbol, $1, $3);
		$$ = divNode;
	 }
	 | expr MOD_OP expr {
		Symbol* modSymbol = makeSymbol("", modOp);
		ASTNode* modNode = makeNode(modSymbol, $1, $3);
		$$ = modNode;
	 }
	 ;

rel_op : expr LESS_OP expr {
		Symbol* lessSymbol = makeSymbol("", lessOp);
		ASTNode* lessNode = makeNode(lessSymbol, $1, $3);
		$$ = lessNode;
	 }
       | expr GREATER_OP expr {
		Symbol* greaterSymbol = makeSymbol("", greaterOp);
		ASTNode* greaterNode = makeNode(greaterSymbol, $1, $3);
		$$ = greaterNode;
	 }
       | expr EQUALS_OP expr {
		Symbol* equalsSymbol = makeSymbol("", equalsOp);
		ASTNode* equalsNode = makeNode(equalsSymbol, $1, $3);
		$$ = equalsNode;
	 }
       ;

cond_op : expr AND_OP expr {
		Symbol* andSymbol = makeSymbol("", andOp);
		ASTNode* andNode = makeNode(andSymbol, $1, $3);
		$$ = andNode;
	 }
        | expr OR_OP expr {
		Symbol* orSymbol = makeSymbol("", orOp);
		ASTNode* orNode = makeNode(orSymbol, $1, $3);
		$$ = orNode;
	 }
        ;

literal : INT {
		Symbol* intSymbol = makeSymbol("", intLiteral);
		int value = atoi($1);
		intSymbol->value.intValue = value;
		intSymbol->hasValue = 1;
		ASTNode* intNode = makeLeaf(intSymbol);
		$$ = intNode;
	}
        | BOOL {
		Symbol* boolSymbol = makeSymbol("", boolLiteral);

		if (strcmp("true", $1) == 0) {
			boolSymbol->value.boolValue = 1;
		} else {
			boolSymbol->value.boolValue = 0;
		}

		boolSymbol->hasValue = 1;
		ASTNode* boolNode = makeLeaf(boolSymbol);
		$$ = boolNode;
	}
        | FLOAT {
		Symbol* floatSymbol = makeSymbol("", floatLiteral);
		int value = atof($1);
		floatSymbol->value.floatValue = value;
		floatSymbol->hasValue = 1;
		ASTNode* floatNode = makeLeaf(floatSymbol);
		$$ = floatNode;
	}
        ;

%%

void yyerror(const char* s) {
	if (outputFilePtr) {
		fprintf(outputFilePtr, "Error sintactico en la linea %d\n", yylineno);
		exit(1);
	}
}
