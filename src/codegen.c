/* ============================================================================
 *  codegen.c — Three-Address Code (TAC) Generator for Lumis Compiler
 * ============================================================================
 *
 *  CSE 314 COMPILER DESIGN CONCEPTS (VIVA / DEFENSE PREPARATION):
 *
 *  1. WHAT IS THREE-ADDRESS CODE (TAC)?
 *     Three-Address Code (TAC) is an Intermediate Representation (IR) technique
 *     where each instruction has at most three operands (typically two sources and
 *     one destination, e.g., `t1 = a + b`). TAC linearizes complex nested AST trees
 *     into a sequence of simple virtual machine operations.
 *
 *  2. REGISTERS & LABELS:
 *     - `new_temp()` generates sequential temporary registers (`t1`, `t2`, `t3`, ...).
 *     - `new_label()` generates sequential control flow jump labels (`L1`, `L2`, ...).
 *
 *  3. CONTROL FLOW LINEARIZATION:
 *     - `if (cond) stmt1 else stmt2` is flattened using `IF_FALSE cond GOTO L_else`,
 *       followed by `stmt1`, `GOTO L_end`, `LABEL L_else`, `stmt2`, `LABEL L_end`.
 *     - `while (cond) stmt` and `for (init; cond; update) stmt` are flattened similarly
 *       using forward and backward conditional jumps.
 * ============================================================================ */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "codegen.h"
#include <stdlib.h>
#include <string.h>

static int temp_counter = 1;
static int label_counter = 1;

static char *new_temp(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", temp_counter++);
    return strdup(buf);
}

static char *new_label(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "L%d", label_counter++);
    return strdup(buf);
}

static const char *binop_symbol(BinOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_LE:  return "<=";
        case OP_GE:  return ">=";
        case OP_AND: return "&&";
        case OP_OR:  return "||";
        default:     return "?";
    }
}

/* Forward declarations */
static char *gen_expr_tac(AstNode *node, FILE *out);
static void gen_stmt_tac(AstNode *node, FILE *out);

/* ---------- TAC Expression Generator Helpers ---------- */

static char *gen_tac_literal(AstNode *node) {
    if (node->data_type == TYPE_STRING) {
        int len = node->string_value ? strlen(node->string_value) : 0;
        char *buf = (char *)malloc(len + 3);
        snprintf(buf, len + 3, "\"%s\"", node->string_value ? node->string_value : "");
        return buf;
    }
    char buf[64];
    switch (node->data_type) {
        case TYPE_INT:   snprintf(buf, sizeof(buf), "%d", node->int_value); break;
        case TYPE_FLOAT: snprintf(buf, sizeof(buf), "%g", node->float_value); break;
        case TYPE_CHAR:  snprintf(buf, sizeof(buf), "'%c'", node->char_value); break;
        case TYPE_BOOL:  snprintf(buf, sizeof(buf), "%s", node->bool_value ? "true" : "false"); break;
        default:         snprintf(buf, sizeof(buf), "0"); break;
    }
    return strdup(buf);
}

static char *gen_tac_binary_op(AstNode *node, FILE *out) {
    char *left = gen_expr_tac(node->children[0], out);
    char *right = gen_expr_tac(node->children[1], out);
    char *temp = new_temp();

    fprintf(out, "    %s = %s %s %s\n", temp, left, binop_symbol(node->binop), right);

    free(left);
    free(right);
    return temp;
}

static char *gen_tac_unary_op(AstNode *node, FILE *out) {
    char *operand = gen_expr_tac(node->children[0], out);
    char *temp = new_temp();

    if (node->unop == OP_NOT) {
        fprintf(out, "    %s = !%s\n", temp, operand);
    } else if (node->unop == OP_NEG) {
        fprintf(out, "    %s = -%s\n", temp, operand);
    }

    free(operand);
    return temp;
}

static char *gen_tac_call(AstNode *node, FILE *out) {
    /* Emit PARAM instructions for function parameters */
    for (int i = 0; i < node->arg_count; i++) {
        char *arg = gen_expr_tac(node->args[i], out);
        fprintf(out, "    PARAM %s\n", arg);
        free(arg);
    }
    char *temp = new_temp();
    fprintf(out, "    %s = CALL %s, %d\n", temp, node->name, node->arg_count);
    return temp;
}

static char *gen_expr_tac(AstNode *node, FILE *out) {
    if (!node) return strdup("");

    switch (node->kind) {
        case NODE_LITERAL:   return gen_tac_literal(node);
        case NODE_VAR_REF:   return strdup(node->name);
        case NODE_BINARY_OP: return gen_tac_binary_op(node, out);
        case NODE_UNARY_OP:  return gen_tac_unary_op(node, out);
        case NODE_CALL:      return gen_tac_call(node, out);
        default:             return strdup("");
    }
}

/* ---------- TAC Statement Generator Helpers ---------- */

static void gen_tac_if(AstNode *node, FILE *out) {
    char *label_else = new_label();
    char *label_end  = new_label();

    char *cond = gen_expr_tac(node->children[0], out);
    fprintf(out, "    IF_FALSE %s GOTO %s\n", cond, (node->child_count > 2) ? label_else : label_end);
    free(cond);

    /* Then branch */
    gen_stmt_tac(node->children[1], out);

    if (node->child_count > 2) {
        fprintf(out, "    GOTO %s\n", label_end);
        fprintf(out, "LABEL %s:\n", label_else);
        /* Else branch */
        gen_stmt_tac(node->children[2], out);
    }

    fprintf(out, "LABEL %s:\n", label_end);

    free(label_else);
    free(label_end);
}

static void gen_tac_while(AstNode *node, FILE *out) {
    char *label_start = new_label();
    char *label_end   = new_label();

    fprintf(out, "LABEL %s:\n", label_start);

    char *cond = gen_expr_tac(node->children[0], out);
    fprintf(out, "    IF_FALSE %s GOTO %s\n", cond, label_end);
    free(cond);

    gen_stmt_tac(node->children[1], out);

    fprintf(out, "    GOTO %s\n", label_start);
    fprintf(out, "LABEL %s:\n", label_end);

    free(label_start);
    free(label_end);
}

static void gen_tac_for(AstNode *node, FILE *out) {
    char *label_start = new_label();
    char *label_end   = new_label();

    /* Init statement */
    gen_stmt_tac(node->children[0], out);

    fprintf(out, "LABEL %s:\n", label_start);

    /* Condition */
    char *cond = gen_expr_tac(node->children[1], out);
    fprintf(out, "    IF_FALSE %s GOTO %s\n", cond, label_end);
    free(cond);

    /* Body */
    gen_stmt_tac(node->children[3], out);

    /* Update statement */
    gen_stmt_tac(node->children[2], out);

    fprintf(out, "    GOTO %s\n", label_start);
    fprintf(out, "LABEL %s:\n", label_end);

    free(label_start);
    free(label_end);
}

static void gen_tac_func_decl(AstNode *node, FILE *out) {
    fprintf(out, "\nFUNC %s:\n", node->name);
    for (int i = 0; i < node->child_count; i++) {
        gen_stmt_tac(node->children[i], out);
    }
    fprintf(out, "    END FUNC\n");
}

static void gen_stmt_tac(AstNode *node, FILE *out) {
    if (!node) return;

    switch (node->kind) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->child_count; i++) {
                gen_stmt_tac(node->children[i], out);
            }
            break;

        case NODE_FUNC_DECL:
            gen_tac_func_decl(node, out);
            break;

        case NODE_VAR_DECL:
            if (node->child_count > 0) {
                char *rhs = gen_expr_tac(node->children[0], out);
                fprintf(out, "    %s = %s\n", node->name, rhs);
                free(rhs);
            }
            break;

        case NODE_ASSIGN: {
            char *rhs = gen_expr_tac(node->children[0], out);
            fprintf(out, "    %s = %s\n", node->name, rhs);
            free(rhs);
            break;
        }

        case NODE_IF:    gen_tac_if(node, out); break;
        case NODE_WHILE: gen_tac_while(node, out); break;
        case NODE_FOR:   gen_tac_for(node, out); break;

        case NODE_RETURN: {
            if (node->child_count > 0) {
                char *val = gen_expr_tac(node->children[0], out);
                fprintf(out, "    RETURN %s\n", val);
                free(val);
            } else {
                fprintf(out, "    RETURN\n");
            }
            break;
        }

        case NODE_PRINT: {
            char *val = gen_expr_tac(node->children[0], out);
            fprintf(out, "    PRINT %s\n", val);
            free(val);
            break;
        }

        case NODE_BLOCK:
            for (int i = 0; i < node->child_count; i++) {
                gen_stmt_tac(node->children[i], out);
            }
            break;

        case NODE_CALL: {
            char *res = gen_expr_tac(node, out);
            free(res);
            break;
        }

        default:
            break;
    }
}

/* Public Entry Point */
void codegen_generate(AstNode *root, FILE *out) {
    temp_counter = 1;
    label_counter = 1;
    gen_stmt_tac(root, out);
}
