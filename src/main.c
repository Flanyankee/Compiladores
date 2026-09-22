#include "cli.h"
#include <stdio.h>
#include "main.h"

int main() {
	takeInput();
	printf("prefix: %s\n", prefix);
	printf("debugFlag: %d\n", returnDebugFlag());
	printf("optimization: %d\n", returnOptimization());
	printf("outputFile: %s\n", returnOutputFile());
	printf("target: %d\n", returnTargetStage());

	return 0;
}
