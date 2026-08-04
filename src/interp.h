/* =============================================================
 *  interp.h — AST Interpreter / Execution Engine for Lumis
 * =============================================================
 *  Executes the validated Lumis AST directly in-memory.
 * ============================================================= */

#ifndef LUMIS_INTERP_H
#define LUMIS_INTERP_H

#include "ast.h"

/* Execute the AST program starting from function 'main'.
   Returns exit status (e.g. 0 on success). */
int interp_execute(AstNode *root);

#endif /* LUMIS_INTERP_H */
