#include "main.h"
#include "cli.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
	handleCliInput(argc, argv);
	printf("debugFlag: %d\n", returnDebugFlag());
	printf("optimization: %d\n", returnOptimization());
	printf("outputFile: %s\n", returnOutputFile());
	printf("sourceFile: %s\n", returnSourceFile());
	printf("target: %d\n", returnTargetStage());

	return 0;
}
