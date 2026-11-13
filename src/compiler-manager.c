#include "compiler-manager.h"
#include "assembly.h"
#include "cli.h"
#include "codinter.h"
#include "constants.h"
#include "optimizations.h"
#include "parser.tab.h"
#include "scanner.h"
#include "semantic-analizer.h"
#include "symbols-tab.h"
#include "syntax-tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int optimizationLevel, target, isDebugEnabled;
char *sourceFileName;
FILE *source;
FILE *outputFilePtr = NULL;

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
	makeOutputFile(targetStage);
	switch (targetStage) {
	case scan:
		scannerStage();
		break;
	case parse:
		parseStage();
		break;
	case semantic:
		semanticStage();
		break;
	case codinter:
		codinterStage();
		break;
	case assembly:
		assemblyStage();
		break;
	default:
		objectStage();
		break;
	}
	fclose(outputFilePtr);
}

void makeOutputFile(int targetStage) {
	char *outputFileName = returnOutputFile();

	switch (targetStage) {
	case scan:
		strcat(outputFileName, ".lex");
		break;
	case parse:
		strcat(outputFileName, ".sint");
		break;
	case semantic:
		strcat(outputFileName, ".sem");
		break;
	case codinter:
		strcat(outputFileName, ".ci");
		break;
	case assembly:
		strcat(outputFileName, ".ass");
		break;
	default:
		strcat(outputFileName, ".out");
		break;
	}

	FILE *out = fopen(outputFileName, "w");
	if (out == NULL) {
		perror("Error");
		printf("\n");
		exit(1);
	}
	outputFilePtr = out;
}

void scannerStage() {
	yyin = source;
	int token;
	while ((token = yylex()) != 0) {
		fprintf(outputFilePtr, "ID: %d, Lexema: %s\n", token, yytext);
	}
}

void parseStage() {
	yyin = source;
	if (yyparse() == 0) {
		if (target == parse) {
			fprintf(outputFilePtr, "Cadena aceptada.\n");
		}
	}
};

int semanticStage() {
	parseStage();
	semanticOutputFile = outputFilePtr;
	symbolsTab = initializeSymbolTab();
	if (analyzeAst(syntaxTree, symbolsTab) == 0) {
		if (target == semantic) {
			fprintf(outputFilePtr,
			        "Cadena aceptada. No se encontraron errores semanticos.\n");
		}
		return 1;
	}
	return 0;
}

void codinterStage() {
	int semanticResult = semanticStage();
	constantOptimizations(optimizationLevel);
	deadCodeOptimizations(optimizationLevel);
	initCodinterGenerator();
	if (semanticResult) {
		generateCodinterFromAST(syntaxTree);
		if (getCodinterListHead() != NULL && target == codinter) {
			printCodinter(outputFilePtr);
		}
	}
}

void assemblyStage() {
	codinterStage();
	generateAssembly(syntaxTree, getCodinterListHead(), outputFilePtr);
}
void objectStage() {}
