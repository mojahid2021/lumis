/* =============================================================
 *  symtab.c — Symbol Table implementation for Lumis
 * =============================================================
 *  See symtab.h for the data layout. Each Scope is a linked
 *  list of Symbol entries, and the table keeps a pointer to
 *  the "current" scope. New symbols are inserted at the head
 *  for O(1) insertion (lookup walks the list — fine for our
 *  tiny programs).
 * ============================================================= */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "symtab.h"
#include <stdlib.h>
#include <string.h>

/* ---------- scope helpers ---------- */

static Scope *new_scope(Scope *parent, const char *name) {
    Scope *s = (Scope *)calloc(1, sizeof(Scope));
    s->parent = parent;
    s->name   = strdup(name);
    return s;
}

/* ---------- lifecycle ---------- */

void symtab_init(SymbolTable *tab) {
    tab->global        = new_scope(NULL, "global");
    tab->current       = tab->global;
    tab->scope_counter = 0;
}

void symtab_free(SymbolTable *tab) {
    /* walk back up freeing each scope */
    Scope *s = tab->current;
    while (s) {
        Scope *parent = s->parent;
        Symbol *sym = s->head;
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            if (sym->param_types) free(sym->param_types);
            free(sym);
            sym = next;
        }
        free(s->name);
        free(s);
        s = parent;
    }
    tab->current = tab->global = NULL;
}

/* ---------- scope entry / exit ---------- */

void symtab_enter_scope(SymbolTable *tab, const char *name) {
    Scope *child = new_scope(tab->current, name);
    tab->current = child;
}

void symtab_leave_scope(SymbolTable *tab) {
    if (tab->current->parent) {
        Scope *old = tab->current;
        tab->current = old->parent;

        /* free the symbols of the popped scope */
        Symbol *sym = old->head;
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
        free(old->name);
        free(old);
    }
}

/* ---------- insertion ---------- */

int symtab_insert(SymbolTable *tab, const char *name, SymKind kind,
                  DataType type, int line) {
    /* Reject duplicates in the *current* scope only. */
    if (symtab_lookup_current(tab, name)) {
        return 0;   /* already declared */
    }
    Symbol *s = (Symbol *)calloc(1, sizeof(Symbol));
    s->name = strdup(name);
    s->kind = kind;
    s->type = type;
    s->line = line;
    s->next = tab->current->head;
    tab->current->head = s;
    return 1;
}

int symtab_insert_func(SymbolTable *tab, const char *name, DataType return_type,
                       int param_count, DataType *param_types, int line) {
    if (symtab_lookup_current(tab, name)) {
        return 0;
    }
    Symbol *s = (Symbol *)calloc(1, sizeof(Symbol));
    s->name = strdup(name);
    s->kind = SYM_FUNCTION;
    s->type = return_type;
    s->line = line;
    s->param_count = param_count;
    if (param_count > 0 && param_types) {
        s->param_types = (DataType *)malloc(param_count * sizeof(DataType));
        memcpy(s->param_types, param_types, param_count * sizeof(DataType));
    }
    s->next = tab->current->head;
    tab->current->head = s;
    return 1;
}

/* ---------- lookup ---------- */

Symbol *symtab_lookup_current(SymbolTable *tab, const char *name) {
    for (Symbol *s = tab->current->head; s; s = s->next) {
        if (strcmp(s->name, name) == 0) return s;
    }
    return NULL;
}

Symbol *symtab_lookup(SymbolTable *tab, const char *name) {
    for (Scope *scope = tab->current; scope; scope = scope->parent) {
        for (Symbol *s = scope->head; s; s = s->next) {
            if (strcmp(s->name, name) == 0) return s;
        }
    }
    return NULL;
}

/* ---------- printing ---------- */

static const char *sym_kind_name(SymKind k) {
    switch (k) {
        case SYM_VARIABLE: return "var";
        case SYM_FUNCTION: return "func";
        case SYM_PARAMETER: return "param";
    }
    return "?";
}

static void print_scope(Scope *s, FILE *out, int indent) {
    for (int i = 0; i < indent; i++) fputc(' ', out);
    fprintf(out, "Scope: %s\n", s->name);

    for (Symbol *sym = s->head; sym; sym = sym->next) {
        for (int i = 0; i < indent + 2; i++) fputc(' ', out);
        fprintf(out, "  %s %s : %s   (line %d)\n",
                sym_kind_name(sym->kind),
                sym->name,
                type_name(sym->type),
                sym->line);
    }
}

void symtab_print(SymbolTable *tab, FILE *out) {
    /* Print global scope first */
    print_scope(tab->global, out, 0);
    /* Then any nested scopes that are still attached */
    /* (Children that have been popped are freed already, so
       this only shows the live stack. For a teaching compiler,
       that's enough.) */
    Scope *s = tab->current;
    while (s && s != tab->global) {
        print_scope(s, out, 0);
        s = s->parent;
    }
}
