/* =============================================================
 *  c_backend.h — C Backend & Binary Compiler for Lumis
 * =============================================================
 *  Translates the AST into target C source code and compiles
 *  it using GCC to produce native executable binaries.
 * ============================================================= */

#ifndef LUMIS_C_BACKEND_H
#define LUMIS_C_BACKEND_H

#include "ast.h"

/* Generates C code from the AST into the specified file stream. */
void c_backend_generate(AstNode *root, FILE *out);

/* Compiles the AST directly into a native executable binary.
   Returns 0 on success. */
int c_backend_compile_binary(AstNode *root, const char *output_binary);

#endif /* LUMIS_C_BACKEND_H */
