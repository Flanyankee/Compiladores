enum optimizations { none };
enum compilerStages { scan, parse, codinter, assembly, object };
enum symbolTypes {
	function,
	global_var,
	local_var,
	formalParameter,

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
	orOp

};
enum types { tInt, tFloat, tBoolean, tVoid, blank };
