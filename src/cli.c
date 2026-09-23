#include "cli.h"
#include "constants.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *optimization = NULL, *outputFile = NULL, *targetStage = NULL,
     *debugFlag = NULL, *helpFlag = NULL, *sourceFile = NULL;

void handleCliInput(int num, char **args) {
	int sourceFileFound = 0;
	for (int i = 0; i < num; i++) {
		if (strcmp(args[i], "-o") == 0) {
			outputFile = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(outputFile, args[i + 1]);
		} else if (strcmp(args[i], "-target") == 0) {
			targetStage = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(targetStage, args[i + 1]);
		} else if (strcmp(args[i], "-opt") == 0) {
			optimization = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(optimization, args[i + 1]);
		} else if (strcmp(args[i], "-debug") == 0) {
			debugFlag = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(debugFlag, args[i + 1]);
		} else if (strcmp(args[i], "-h") == 0 ||
		           strcmp(args[i], "-help") == 0) {
			helpFlag = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(helpFlag, args[i]);
		} else if (strstr(args[i], ".ctds")) {
			sourceFileFound = 1;
			sourceFile = (char *)malloc(sizeof(args[i + 1] + 1));
			strcpy(sourceFile, args[i]);
		}
	}
	if (!sourceFileFound && !helpFlag) {
		printf("Error: %s. (Error code %d)\n", strerror(1), 1);
		exit(2);
	}
	if (helpFlag) {
		printHelp();
		exit(0);
	}
}

int returnOptimization() {
	if (!optimization) {
		return -1;
	}

	if (strcmp(optimization, "none") == 0) {
		return -1;
	} else {
		return 9999;
	}
}

int returnTargetStage() {
	if (!targetStage) {
		return object;
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

char *returnOutputFile() {
	char *nameCopy = (char *)malloc(sizeof(char));
	if (outputFile) {
		strcpy(nameCopy, outputFile);
		return nameCopy;
	} else {
		strcpy(nameCopy, "outputFile");
		return nameCopy;
	}
}

char *returnSourceFile() {
	char *nameCopy = (char *)malloc(sizeof(char));
	strcpy(nameCopy, sourceFile);
	return nameCopy;
}

void printHelp() {
	printf("Usage: c-tds [-target STAGE] [-o NAME] [-opt OPTIMIZ] [-debug] "
	       "file.ctds\n");
	printf("Options:\n");
	printf("-o NAME\t\tRename the executable file to NAME\n");
	printf("\n-target STAGE\tCompilation proceeds up to STAGE\n");
	printf("\t\tor generates an executable file with .out extension\n");
	printf("\t\tif no target is specified.\n");
	printf("\t\tAvailable stages:\n");
	printf("\t\t\tscan\t\t Generates a scanner whose filename ends in .lex\n");
	printf("\t\t\tparse\t\t Generates a parser whose filename ends in .<falta "
	       "completar>\n");
	printf("\t\t\tcodinter\t Generates an interpreter whose filename ends in "
	       ".<falta completar>\n");
	printf("\t\t\tassembly\t Generates assembly code whose filename ends in "
	       ".<falta completar>\n");
	printf("\n-opt OPTIMZ\tPerform the specified optimizations.\n");
	printf("\t\tno optimization is performed\n");
	printf("\t\tunless one is specified.\n");
	printf("\t\tAvailable optimizations:\n");
	printf("\t\t\tnone\t\t Performs no optimization.\n");
	printf("\n-debug\t\tPrint debug information.\n");
}
