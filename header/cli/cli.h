
#ifndef CLI_H
#define CLI_H

extern char *prefix, *optimization, *outputFile, *targetStage, *debugFlag,
    *helpFlag;

void takeInput();

/**
 * Returns the optimization type.
 * If no optimization was set, it returns -1.
 */
int returnOptimization();

/**
 * Returns the target stage.
 * If no target has been specified, it returns the most advanced state
 * available.
 */
int returnTargetStage();

/**
 * Returns 1 if the debug flags have been enabled.
 * Otherwise returns 0.
 */
int returnDebugFlag();

/**
 * Returns a copy of the specified output file name.
 */
char *returnOutputFile();

void printHelp();
#endif // CLI_H
