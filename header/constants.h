#ifndef CONSTANTS_H
#define CONSTANTS_H
enum optimizations { none };
enum compilerStages { scan, parse, codinter, assembly, object };
enum symbolTypes {
	function,
	global_var,
	local_var,
	formalParameter,

	program,
	constant, // despues hay que ver si borramos constant o borramos los
	          // literals de abajo.
	variable,
	idList,
	functionInvoke,
	formalParameterList,
	actualParameterList,
	actualParameter,
	block,
	statementList,
	ifNode,
	elseNode,
	whileNode,
	returnNode,

	varDeclarationList,
	varDeclaration,
	methodDeclarationList,
	methodDeclaration,
	addOp,
	substractOp,
	multOp,
	divOp,
	modOp,
	lessOp,
	greaterOp,
	equalsOp,
	andOp,
	orOp,
	negationOp,
	assignOp,

	intLiteral,
	boolLiteral,
	floatLiteral,

	blankNode
};
enum types { tInt, tFloat, tBoolean, tVoid, blank };

#endif
