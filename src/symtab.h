/* =============================================================
 *  symtab.h — Symbol Table for Lumis
 * =============================================================
 *
 *  A symbol table maps identifiers (variable / function names)
 *  to their declared type, scope, and other attributes. The
 *  semantic checker uses it to answer questions like:
 *      "Is this variable declared?"
 *      "Has it been declared already in this scope?"
 *      "Does this function exist?"
 *
 *  Implementation: a simple linked list of scopes, each of
 *  which is a linked list of symbols. Easy to read, good enough
 *  for a teaching compiler.
 * ============================================================= */

#ifndef LUMIS_SYMTAB_H
#define LUMIS_SYMTAB_H

#include "ast.h"

/* ---------- Symbol Kinds ---------- */
typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_PARAMETER   /* treated like a variable, but distinguishable */
} SymKind;

/* ---------- A single symbol entry ---------- */
typedef struct Symbol {
    char       *name;       /* identifier text */
    SymKind     kind;       /* variable / function / parameter */
    DataType    type;       /* int / float / char / bool / void */
    int         line;       /* where it was declared */
    int         param_count;/* for SYM_FUNCTION: parameter count */
    DataType   *param_types;/* for SYM_FUNCTION: parameter types */
    struct Symbol *next;    /* next symbol in the same scope */
} Symbol;

/* ---------- A scope (linked list of symbols + parent pointer) ---------- */
typedef struct Scope {
    Symbol        *head;     /* first symbol in this scope */
    struct Scope  *parent;   /* enclosing scope (NULL = global) */
    char          *name;     /* for debugging ("global", "main", "block-3") */
} Scope;

/* ---------- The symbol table itself ---------- */
typedef struct {
    Scope *current;          /* the scope we're currently in */
    Scope *global;           /* the root scope, kept for printing */
    int    scope_counter;    /* used to give scopes unique names */
} SymbolTable;

/* ---------- API ---------- */

/* Initialize a fresh symbol table with one global scope. */
void symtab_init(SymbolTable *tab);

/* Free all memory owned by the symbol table. */
void symtab_free(SymbolTable *tab);

/* Enter a new scope (called when entering a function body or
   a `{ ... }` block). `name` is a human-readable label. */
void symtab_enter_scope(SymbolTable *tab, const char *name);

/* Leave the current scope (called at the matching `}`). */
void symtab_leave_scope(SymbolTable *tab);

/* Insert a symbol into the current scope.
   Returns 1 on success, 0 if a symbol with the same name
   already exists in the current scope. */
int symtab_insert(SymbolTable *tab, const char *name, SymKind kind,
                  DataType type, int line);

/* Insert a function signature into the current scope. */
int symtab_insert_func(SymbolTable *tab, const char *name, DataType return_type,
                       int param_count, DataType *param_types, int line);

/* Look up a name by walking up the scope chain.
   Returns the closest matching symbol, or NULL if not found. */
Symbol *symtab_lookup(SymbolTable *tab, const char *name);

/* Look up a name only in the current scope (no parent walk).
   Used to detect redeclaration. */
Symbol *symtab_lookup_current(SymbolTable *tab, const char *name);

/* Pretty-print the entire symbol table to `out`. */
void symtab_print(SymbolTable *tab, FILE *out);

#endif /* LUMIS_SYMTAB_H */
