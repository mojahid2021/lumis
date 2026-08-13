/* =============================================================
 *  semantic.h — Semantic Analyzer for Lumis
 * ============================================================= */

#ifndef LUMIS_SEMANTIC_H
#define LUMIS_SEMANTIC_H

#include "ast.h"
#include "symtab.h"

/* Analyzes the given AST against the symbol table.
   Returns the number of semantic errors encountered (0 = success). */
int semantic_check(AstNode *root, SymbolTable *symtab);

#endif /* LUMIS_SEMANTIC_H */
