#include "main.h"
#include "cli.h"
#include "compiler-manager.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
	handleCliInput(argc, argv);
	manageCompilation();

	return 0;
}
