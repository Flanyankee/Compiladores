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
	variable,
	idList,
	functionInvoke,
	actualParameterList,
	actualParameter,
	block,
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
	assingOp

};
enum types { tInt, tFloat, tBoolean, tVoid, blank };

#endif
