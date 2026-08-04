/* =============================================================
 *  interp.c — AST Interpreter / Execution Engine for Lumis
 * ============================================================= */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "interp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Value {
    DataType type;
    union {
        int int_val;
        double float_val;
        char char_val;
        int bool_val;
    };
} Value;

typedef struct VarSymbol {
    char *name;
    Value val;
    struct VarSymbol *next;
} VarSymbol;

typedef struct Env {
    VarSymbol *vars;
    struct Env *parent;
} Env;

typedef struct InterpState {
    AstNode *root;
    int returning;
    Value return_val;
} InterpState;

static Env *env_create(Env *parent) {
    Env *env = (Env *)calloc(1, sizeof(Env));
    env->parent = parent;
    return env;
}

static void env_free(Env *env) {
    if (!env) return;
    VarSymbol *curr = env->vars;
    while (curr) {
        VarSymbol *tmp = curr;
        curr = curr->next;
        free(tmp->name);
        free(tmp);
    }
    free(env);
}

static void env_define(Env *env, const char *name, Value val) {
    VarSymbol *s = (VarSymbol *)malloc(sizeof(VarSymbol));
    s->name = strdup(name);
    s->val = val;
    s->next = env->vars;
    env->vars = s;
}

static VarSymbol *env_lookup_sym(Env *env, const char *name) {
    for (Env *e = env; e != NULL; e = e->parent) {
        for (VarSymbol *s = e->vars; s != NULL; s = s->next) {
            if (strcmp(s->name, name) == 0) return s;
        }
    }
    return NULL;
}

static Value env_get(Env *env, const char *name) {
    VarSymbol *s = env_lookup_sym(env, name);
    if (s) return s->val;
    Value v = { .type = TYPE_INT, .int_val = 0 };
    return v;
}

static void env_set(Env *env, const char *name, Value val) {
    VarSymbol *s = env_lookup_sym(env, name);
    if (s) {
        s->val = val;
    } else {
        env_define(env, name, val);
    }
}

static AstNode *find_function(AstNode *root, const char *name) {
    if (!root || root->kind != NODE_PROGRAM) return NULL;
    for (int i = 0; i < root->child_count; i++) {
        AstNode *child = root->children[i];
        if (child->kind == NODE_FUNC_DECL && strcmp(child->name, name) == 0) {
            return child;
        }
    }
    return NULL;
}

static Value eval_expr(AstNode *node, Env *env, InterpState *st);
static void exec_stmt(AstNode *node, Env *env, InterpState *st);

static Value eval_expr(AstNode *node, Env *env, InterpState *st) {
    Value res = { .type = TYPE_INT, .int_val = 0 };
    if (!node) return res;

    switch (node->kind) {
        case NODE_LITERAL:
            res.type = node->data_type;
            switch (node->data_type) {
                case TYPE_INT:   res.int_val   = node->int_value; break;
                case TYPE_FLOAT: res.float_val = node->float_value; break;
                case TYPE_CHAR:  res.char_val  = node->char_value; break;
                case TYPE_BOOL:  res.bool_val  = node->bool_value; break;
                default: break;
            }
            return res;

        case NODE_VAR_REF:
            return env_get(env, node->name);

        case NODE_UNARY_OP: {
            Value sub = eval_expr(node->children[0], env, st);
            if (node->unop == OP_NEG) {
                if (sub.type == TYPE_FLOAT) sub.float_val = -sub.float_val;
                else sub.int_val = -sub.int_val;
            } else if (node->unop == OP_NOT) {
                int b = (sub.type == TYPE_BOOL) ? sub.bool_val : (sub.type == TYPE_FLOAT ? (sub.float_val != 0) : (sub.int_val != 0));
                sub.type = TYPE_BOOL;
                sub.bool_val = !b;
            }
            return sub;
        }

        case NODE_BINARY_OP: {
            Value l = eval_expr(node->children[0], env, st);
            Value r = eval_expr(node->children[1], env, st);

            int is_float = (l.type == TYPE_FLOAT || r.type == TYPE_FLOAT);
            double lf = (l.type == TYPE_FLOAT) ? l.float_val : (l.type == TYPE_CHAR ? l.char_val : l.int_val);
            double rf = (r.type == TYPE_FLOAT) ? r.float_val : (r.type == TYPE_CHAR ? r.char_val : r.int_val);

            switch (node->binop) {
                case OP_ADD:
                    if (is_float) { res.type = TYPE_FLOAT; res.float_val = lf + rf; }
                    else { res.type = TYPE_INT; res.int_val = (int)lf + (int)rf; }
                    break;
                case OP_SUB:
                    if (is_float) { res.type = TYPE_FLOAT; res.float_val = lf - rf; }
                    else { res.type = TYPE_INT; res.int_val = (int)lf - (int)rf; }
                    break;
                case OP_MUL:
                    if (is_float) { res.type = TYPE_FLOAT; res.float_val = lf * rf; }
                    else { res.type = TYPE_INT; res.int_val = (int)lf * (int)rf; }
                    break;
                case OP_DIV:
                    if (is_float) { res.type = TYPE_FLOAT; res.float_val = rf != 0 ? lf / rf : 0; }
                    else { res.type = TYPE_INT; res.int_val = rf != 0 ? (int)lf / (int)rf : 0; }
                    break;
                case OP_MOD:
                    res.type = TYPE_INT;
                    res.int_val = ((int)rf != 0) ? ((int)lf % (int)rf) : 0;
                    break;

                case OP_EQ:  res.type = TYPE_BOOL; res.bool_val = (lf == rf); break;
                case OP_NEQ: res.type = TYPE_BOOL; res.bool_val = (lf != rf); break;
                case OP_LT:  res.type = TYPE_BOOL; res.bool_val = (lf < rf); break;
                case OP_GT:  res.type = TYPE_BOOL; res.bool_val = (lf > rf); break;
                case OP_LE:  res.type = TYPE_BOOL; res.bool_val = (lf <= rf); break;
                case OP_GE:  res.type = TYPE_BOOL; res.bool_val = (lf >= rf); break;

                case OP_AND: res.type = TYPE_BOOL; res.bool_val = (lf != 0) && (rf != 0); break;
                case OP_OR:  res.type = TYPE_BOOL; res.bool_val = (lf != 0) || (rf != 0); break;
            }
            return res;
        }

        case NODE_CALL: {
            AstNode *fdecl = find_function(st->root, node->name);
            if (!fdecl) return res;

            Env *fenv = env_create(NULL);
            for (int i = 0; i < node->arg_count && i < fdecl->param_count; i++) {
                Value arg_val = eval_expr(node->args[i], env, st);
                env_define(fenv, fdecl->params[i]->name, arg_val);
            }

            int prev_ret = st->returning;
            Value prev_val = st->return_val;

            st->returning = 0;
            for (int i = 0; i < fdecl->child_count; i++) {
                exec_stmt(fdecl->children[i], fenv, st);
                if (st->returning) break;
            }

            Value ret = st->return_val;

            st->returning = prev_ret;
            st->return_val = prev_val;

            env_free(fenv);
            return ret;
        }

        default:
            return res;
    }
}

static void exec_stmt(AstNode *node, Env *env, InterpState *st) {
    if (!node || st->returning) return;

    switch (node->kind) {
        case NODE_BLOCK: {
            Env *local = env_create(env);
            for (int i = 0; i < node->child_count; i++) {
                exec_stmt(node->children[i], local, st);
                if (st->returning) break;
            }
            env_free(local);
            break;
        }

        case NODE_VAR_DECL: {
            Value init = { .type = node->data_type, .int_val = 0 };
            if (node->child_count > 0) {
                init = eval_expr(node->children[0], env, st);
            }
            env_define(env, node->name, init);
            break;
        }

        case NODE_ASSIGN: {
            Value val = eval_expr(node->children[0], env, st);
            env_set(env, node->name, val);
            break;
        }

        case NODE_IF: {
            Value cond = eval_expr(node->children[0], env, st);
            int cond_true = (cond.type == TYPE_BOOL) ? cond.bool_val : (cond.type == TYPE_FLOAT ? (cond.float_val != 0) : (cond.int_val != 0));
            if (cond_true) {
                exec_stmt(node->children[1], env, st);
            } else if (node->child_count > 2) {
                exec_stmt(node->children[2], env, st);
            }
            break;
        }

        case NODE_WHILE: {
            while (!st->returning) {
                Value cond = eval_expr(node->children[0], env, st);
                int cond_true = (cond.type == TYPE_BOOL) ? cond.bool_val : (cond.type == TYPE_FLOAT ? (cond.float_val != 0) : (cond.int_val != 0));
                if (!cond_true) break;
                exec_stmt(node->children[1], env, st);
            }
            break;
        }

        case NODE_FOR: {
            Env *for_env = env_create(env);
            exec_stmt(node->children[0], for_env, st); /* init */

            while (!st->returning) {
                Value cond = eval_expr(node->children[1], for_env, st); /* cond */
                int cond_true = (cond.type == TYPE_BOOL) ? cond.bool_val : (cond.type == TYPE_FLOAT ? (cond.float_val != 0) : (cond.int_val != 0));
                if (!cond_true) break;

                exec_stmt(node->children[3], for_env, st); /* body */
                if (st->returning) break;

                exec_stmt(node->children[2], for_env, st); /* update */
            }

            env_free(for_env);
            break;
        }

        case NODE_RETURN: {
            if (node->child_count > 0) {
                st->return_val = eval_expr(node->children[0], env, st);
            } else {
                st->return_val = (Value){ .type = TYPE_INT, .int_val = 0 };
            }
            st->returning = 1;
            break;
        }

        case NODE_PRINT: {
            Value val = eval_expr(node->children[0], env, st);
            switch (val.type) {
                case TYPE_INT:   printf("%d\n", val.int_val); break;
                case TYPE_FLOAT: printf("%g\n", val.float_val); break;
                case TYPE_CHAR:  printf("%c\n", val.char_val); break;
                case TYPE_BOOL:  printf("%s\n", val.bool_val ? "true" : "false"); break;
                default:         printf("%d\n", val.int_val); break;
            }
            break;
        }

        case NODE_CALL:
            eval_expr(node, env, st);
            break;

        default:
            break;
    }
}

int interp_execute(AstNode *root) {
    if (!root) return 1;

    AstNode *main_func = find_function(root, "main");
    if (!main_func) {
        fprintf(stderr, "Error: No 'main' function found in program.\n");
        return 1;
    }

    InterpState state = { .root = root, .returning = 0, .return_val = { .type = TYPE_INT, .int_val = 0 } };
    Env *global_env = env_create(NULL);

    for (int i = 0; i < main_func->child_count; i++) {
        exec_stmt(main_func->children[i], global_env, &state);
        if (state.returning) break;
    }

    int exit_code = (state.return_val.type == TYPE_INT) ? state.return_val.int_val : 0;
    env_free(global_env);
    return exit_code;
}
