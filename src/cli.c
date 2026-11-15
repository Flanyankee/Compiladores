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
		return none;
	}

	if (strcmp(optimization, "none") == 0) {
		return none;
	} else if (strcmp(optimization, "constants") == 0) {
		return constants;
	} else if (strcmp(optimization, "dead-code") == 0) {
		return dead_code;
	} else {
		return all;
	}
}

int returnTargetStage() {
	if (!targetStage) {
		return assembly;
	}

	if (strcmp(targetStage, "scan") == 0) {
		return scan;
	} else if (strcmp(targetStage, "parse") == 0) {
		return parse;
	} else if (strcmp(targetStage, "semantic") == 0) {
		return semantic;
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
	if (outputFile) {
		char *nameCopy = (char *)malloc(strlen(outputFile) + 1);
		strcpy(nameCopy, outputFile);
		return nameCopy;
	} else {
		char *nameCopy = (char *)malloc(strlen("outputFile") + 1);
		strcpy(nameCopy, "outputFile");
		return nameCopy;
	}
}

char *returnSourceFile() {
	char *nameCopy = (char *)malloc(strlen(sourceFile) + 1);
	strcpy(nameCopy, sourceFile);
	return nameCopy;
}

void printHelp() {
    printf("Usage: c-tds [-target STAGE] [-o NAME] [-opt OPTIMIZ] [-debug] [-h|-help] file.ctds\n");
    printf("\nOptions:\n");
    printf("  -o NAME\t\tSet the base output filename to NAME (default: outputFile).\n");

    printf("\n  -target STAGE\t\tCompilation proceeds up to STAGE.\n");
    printf("\t\t\tIf no target is specified, compilation proceeds up to\n");
    printf("\t\t\tthe assembly stage (.ass) by default.\n");
    printf("\t\t\tAvailable stages:\n");
    printf("\t\t\t  scan\t\tPerforms lexical analysis; output ends in .lex\n");
    printf("\t\t\t  parse\t\tPerforms syntax analysis; output ends in .sint\n");
    printf("\t\t\t  semantic\tPerforms semantic analysis; output ends in .sem\n");
    printf("\t\t\t  codinter\tGenerates Three-Address Code (TAC); output ends in .ci\n");
    printf("\t\t\t  assembly\tGenerates x86-64 assembly code; output ends in .ass\n");

    printf("\n  -opt OPTIMIZATION\tPerform the specified optimizations before code generation.\n");
    printf("\t\t\tNo optimization is performed unless one is specified.\n");
    printf("\t\t\tAvailable optimizations:\n");
    printf("\t\t\t  none\t\tPerforms no optimization (default).\n");
    printf("\t\t\t  constants\tPerforms constant folding on arithmetic and logical expressions.\n");
    printf("\t\t\t  dead-code\tRemoves unreachable statements after return and false blocks.\n");
    printf("\t\t\t  all\t\tApplies both constant folding and dead-code elimination.\n");

    printf("\n  -debug\t\tEnable debug information.\n");
    printf("  -h, -help\t\tDisplay this help message and exit.\n");
}
