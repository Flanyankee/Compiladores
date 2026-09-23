
#ifndef COMPILER_MANAGER_H
#define COMPILER_MANAGER_H

/**
 * A method responsible for managing the compilation.
 * i.e: Until the target is compiled, applying optimizations, writing the output
 * 	file, etc.
 */
void manageCompilation();

void executeStage(int targetStage);
void applyOptimization(int optimizationLevel);

void scannerStage();
void parseStage();
void codinterStage();
void assemblyStage();
void objectStage();

#endif // COMPILER_MANAGER_H
