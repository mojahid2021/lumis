/* =============================================================
 *  codegen.h — Three-Address Code (TAC) Generator for Lumis
 * =============================================================
 *  Traverses the validated AST and emits human-readable
 *  Three-Address Code (TAC) instructions to stdout or a file.
 * ============================================================= */

#ifndef LUMIS_CODEGEN_H
#define LUMIS_CODEGEN_H

#include <stdio.h>
#include "ast.h"

/* Generates Three-Address Code for the AST and prints to `out`. */
void codegen_generate(AstNode *root, FILE *out);

#endif /* LUMIS_CODEGEN_H */
