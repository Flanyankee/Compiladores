#include "compiler-manager.h"
#include "cli.h"
#include "constants.h"
#include "parser.tab.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int optimizationLevel, target, isDebugEnabled;
char *sourceFileName;
FILE *source;

void manageCompilation() {
	optimizationLevel = returnOptimization();
	target = returnTargetStage();
	isDebugEnabled = returnDebugFlag();
	sourceFileName = returnSourceFile();

	source = fopen(sourceFileName, "r");
	if (source == NULL) {
		perror("Error");
		printf("\n");
		exit(1);
	}

	executeStage(target);
	fclose(source);
}

void executeStage(int targetStage) {
	switch (targetStage) {
	case scan:
		scannerStage();
		break;
	case parse:
		parseStage();
		break;
	case codinter:
		codinterStage();
		break;
	case assembly:
		assemblyStage();
		applyOptimization(optimizationLevel);
		break;
	default:
		objectStage();
		applyOptimization(optimizationLevel);
		break;
	}
}

void applyOptimization(int optimizationLevel) {
	if (optimizationLevel != none) {
	}
}

void scannerStage() {
	char *outputFileName = returnOutputFile();
	strcat(outputFileName, ".lex");
	FILE *out = fopen(outputFileName, "w");
	if (out == NULL) {
		perror("Error");
		printf("\n");
		exit(1);
	}

	yyin = source;

	int token;
	while ((token = yylex()) != 0) {
		fprintf(out, "ID: %d, Lexema: %s\n", token, yytext);
	}
	fclose(out);
}

void parseStage() {
	char *outputFileName = returnOutputFile();
	strcat(outputFileName, ".sint");
	FILE *out = fopen(outputFileName, "w");
	if (out == NULL) {
		perror("Error");
		printf("\n");
		exit(1);
	}

	yyin = source;
	if (yyparse() == 0) {
		fprintf(out, "Cadena aceptada.\n");
	} else {
		fprintf(out, "Syntax error in line %d\n", yylineno);
	}
	fclose(out);
};
void codinterStage() {};
void assemblyStage() {};
void objectStage() {};
