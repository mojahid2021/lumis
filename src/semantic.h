/* =============================================================
 *  semantic.h — Semantic Analyzer for Lumis
 * =============================================================
 *  Traverses the AST built by the parser and enforces semantic rules:
 *    1. Declaration before use: variables and functions must be declared.
 *    2. Scope & redeclaration: no duplicate symbol in the same scope.
 *    3. Type checking: assignments, operations, return values must have compatible types.
 *    4. Function call validation: argument count and types must match signature.
 * ============================================================= */

#ifndef LUMIS_SEMANTIC_H
#define LUMIS_SEMANTIC_H

#include "ast.h"
#include "symtab.h"

/* Analyzes the given AST against the symbol table.
   Returns the number of semantic errors encountered (0 = success). */
int semantic_check(AstNode *root, SymbolTable *symtab);

#endif /* LUMIS_SEMANTIC_H */
