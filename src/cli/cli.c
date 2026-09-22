#include "cli.h"
#include "cli.tab.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *prefix = NULL, *optimization = NULL, *outputFile = NULL, *targetStage = NULL, *debugFlag = NULL, *helpFlag = NULL;
extern int yyparse();

void takeInput() {
	int scannerResult = yyparse();
	if (helpFlag) {
		printHelp();
	}
}

int returnOptimization() {
	if (!optimization) {
		return -1;
	}

	if (strcmp(optimization, "none") == 0) {
		return -1;
	} else if (strcmp(optimization, "death-code") == 0) {
		return deathCode;
	} else {
		return 9999;
	}
}

int returnTargetStage() {
	if (!targetStage) {
		return parse;
	}

	if (strcmp(targetStage, "scan") == 0) {
		return scan;
	} else if (strcmp(targetStage, "parse") == 0) {
		return parse;
	} else if (strcmp(targetStage, "codinter") == 0) {
		return codinter;
	} else {
		return assembly;
	} 
}

int returnDebugFlag() {
	if (debugFlag) {
		return 1;
	} else {
		return 0;
	}
}

char* returnOutputFile() {
	if (outputFile) {
		char* nameCopy = (char*)malloc(sizeof(char));
		strcpy(nameCopy, outputFile);
		return nameCopy;
	} else {
		return "outputFile";
	}
}

void printHelp() {
	if (helpFlag) {
		printf("Usage: c-tds [option] file.ctds\n");
		printf("Options:\n");
		printf("-o NAME\t\tRename the executable file to NAME\n");
		printf("\n-target STAGE\tCompilation proceeds up to STAGE\n");
		printf("\t\tor generates an executable file with .out extension\n");
		printf("\t\tif no target is specified.\n");
		printf("\t\tAvailable stages:\n");
		printf("\t\t\tscan\t\t Generates a scanner whose filename ends in .lex\n");
		printf("\t\t\tparse\t\t Generates a parser whose filename ends in .<falta completar>\n");
		printf("\t\t\tcodinter\t Generates an interpreter whose filename ends in .<falta completar>\n");
		printf("\t\t\tassembly\t Generates assembly code whose filename ends in .<falta completar>\n");
		printf("\n-opt OPTIMZ\tPerform the specified optimizations.\n");
		printf("\t\tno optimization is performed\n");
		printf("\t\tunless one is specified.\n");
		printf("\t\tAvailable optimizations:\n");
		printf("\t\t\tnone\t\t Performs no optimization.\n");
		printf("\n-debug\t\tPrint debug information.\n");
	}
}
