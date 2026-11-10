#ifndef CONSTANTS_H
#define CONSTANTS_H
enum optimizations { none, constants, dead_code, memory, all };
enum compilerStages { scan, parse, semantic, codinter, assembly, object };
enum symbolTypes {
	function,
	global_var,
	local_var,
	formalParameter,

	program,
	variable,
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
