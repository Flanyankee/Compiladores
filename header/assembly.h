#ifndef ASSEMBLY_H
#define ASSEMBLY_H

#include "codinter.h"
#include "syntax-tree.h"
#include <stdio.h>

void generateAssembly(ASTNode *astRoot, CodinterInstruction *codinterHead, FILE *out);

#endif