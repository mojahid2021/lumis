/* =============================================================
 *  ast.c — Abstract Syntax Tree implementation for Lumis
 * ============================================================= */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "ast.h"
#include <stdlib.h>
#include <string.h>

/* ---------- helper: printable type name ---------- */

const char *type_name(DataType t) {
    switch (t) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "float";
        case TYPE_CHAR:   return "char";
        case TYPE_BOOL:   return "bool";
        case TYPE_STRING: return "string";
        case TYPE_VOID:   return "void";
        default:          return "<unknown>";
    }
}

/* ---------- helper: allocate a zeroed node ---------- */

static AstNode *make_node(NodeKind kind, int line) {
    AstNode *n = (AstNode *)calloc(1, sizeof(AstNode));
    n->kind = kind;
    n->line = line;
    return n;
}

/* ---------- helper: dynamic children array ---------- */

void ast_add_child(AstNode *parent, AstNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity = (parent->child_capacity == 0) ? 4 : parent->child_capacity * 2;
        parent->children = (AstNode **)realloc(
            parent->children,
            parent->child_capacity * sizeof(AstNode *)
        );
    }
    parent->children[parent->child_count++] = child;
}

void ast_set_type(AstNode *n, DataType t) {
    n->data_type = t;
}

/* ---------- constructors: top-level ---------- */

AstNode *ast_new_program(int line) {
    return make_node(NODE_PROGRAM, line);
}

AstNode *ast_new_func_decl(int line, char *name, DataType return_type) {
    AstNode *n = make_node(NODE_FUNC_DECL, line);
    n->name        = strdup(name);
    n->return_type = return_type;
    return n;
}

/* ---------- constructors: statements ---------- */

AstNode *ast_new_var_decl(int line, char *name, DataType type) {
    AstNode *n = make_node(NODE_VAR_DECL, line);
    n->name      = strdup(name);
    n->data_type = type;
    return n;
}

AstNode *ast_new_assign(int line, char *name, AstNode *value) {
    AstNode *n = make_node(NODE_ASSIGN, line);
    n->name = strdup(name);
    ast_add_child(n, value);                 /* child[0] = RHS */
    return n;
}

AstNode *ast_new_if(int line, AstNode *cond, AstNode *then_branch, AstNode *else_branch) {
    AstNode *n = make_node(NODE_IF, line);
    ast_add_child(n, cond);                  /* child[0] = condition */
    ast_add_child(n, then_branch);           /* child[1] = then      */
    if (else_branch) ast_add_child(n, else_branch); /* child[2] = else (optional) */
    return n;
}

AstNode *ast_new_while(int line, AstNode *cond, AstNode *body) {
    AstNode *n = make_node(NODE_WHILE, line);
    ast_add_child(n, cond);
    ast_add_child(n, body);
    return n;
}

AstNode *ast_new_for(int line, AstNode *init, AstNode *cond, AstNode *update, AstNode *body) {
    AstNode *n = make_node(NODE_FOR, line);
    ast_add_child(n, init);
    ast_add_child(n, cond);
    ast_add_child(n, update);
    ast_add_child(n, body);
    return n;
}

AstNode *ast_new_return(int line, AstNode *value) {
    AstNode *n = make_node(NODE_RETURN, line);
    if (value) ast_add_child(n, value);
    return n;
}

AstNode *ast_new_print(int line, AstNode *expr) {
    AstNode *n = make_node(NODE_PRINT, line);
    ast_add_child(n, expr);
    return n;
}

AstNode *ast_new_block(int line) {
    return make_node(NODE_BLOCK, line);
}

/* ---------- constructors: expressions ---------- */

AstNode *ast_new_binary(int line, BinOp op, AstNode *lhs, AstNode *rhs) {
    AstNode *n = make_node(NODE_BINARY_OP, line);
    n->binop = op;
    ast_add_child(n, lhs);
    ast_add_child(n, rhs);
    return n;
}

AstNode *ast_new_unary(int line, UnaryOp op, AstNode *operand) {
    AstNode *n = make_node(NODE_UNARY_OP, line);
    n->unop = op;
    ast_add_child(n, operand);
    return n;
}

AstNode *ast_new_call(int line, char *name) {
    AstNode *n = make_node(NODE_CALL, line);
    n->name = strdup(name);
    return n;
}

AstNode *ast_new_literal_int(int line, int value) {
    AstNode *n = make_node(NODE_LITERAL, line);
    n->data_type = TYPE_INT;
    n->int_value = value;
    return n;
}

AstNode *ast_new_literal_float(int line, double value) {
    AstNode *n = make_node(NODE_LITERAL, line);
    n->data_type = TYPE_FLOAT;
    n->float_value = value;
    return n;
}

AstNode *ast_new_literal_char(int line, char value) {
    AstNode *n = make_node(NODE_LITERAL, line);
    n->data_type = TYPE_CHAR;
    n->char_value = value;
    return n;
}

AstNode *ast_new_literal_bool(int line, int value) {
    AstNode *n = make_node(NODE_LITERAL, line);
    n->data_type = TYPE_BOOL;
    n->bool_value = (value != 0);
    return n;
}

AstNode *ast_new_literal_string(int line, const char *value) {
    AstNode *n = make_node(NODE_LITERAL, line);
    n->data_type = TYPE_STRING;
    n->string_value = strdup(value ? value : "");
    return n;
}

AstNode *ast_new_var_ref(int line, char *name) {
    AstNode *n = make_node(NODE_VAR_REF, line);
    n->name = strdup(name);
    return n;
}

/* ---------- argument / parameter helpers ---------- */

void ast_add_arg(AstNode *call, AstNode *arg) {
    call->args = (AstNode **)realloc(call->args, (call->arg_count + 1) * sizeof(AstNode *));
    call->args[call->arg_count++] = arg;
}

void ast_add_param(AstNode *func, AstNode *param) {
    func->params = (AstNode **)realloc(func->params, (func->param_count + 1) * sizeof(AstNode *));
    func->params[func->param_count++] = param;
}

/* ---------- pretty-printing ---------- */

static const char *binop_name(BinOp op) {
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

static const char *unop_name(UnaryOp op) {
    switch (op) {
        case OP_NOT: return "!";
        case OP_NEG: return "-";
        default:     return "?";
    }
}

static void print_indent(FILE *out, int indent) {
    for (int i = 0; i < indent; i++) fputc(' ', out);
}

void ast_print(AstNode *node, FILE *out, int indent) {
    if (!node) return;
    print_indent(out, indent);

    switch (node->kind) {
        case NODE_PROGRAM:
            fprintf(out, "Program\n");
            break;
        case NODE_FUNC_DECL:
            fprintf(out, "FunctionDecl: %s -> %s\n",
                    node->name, type_name(node->return_type));
            break;
        case NODE_VAR_DECL:
            fprintf(out, "VarDecl: %s : %s\n", node->name, type_name(node->data_type));
            break;
        case NODE_ASSIGN:
            fprintf(out, "Assign: %s\n", node->name);
            break;
        case NODE_IF:
            fprintf(out, "If\n");
            break;
        case NODE_WHILE:
            fprintf(out, "While\n");
            break;
        case NODE_FOR:
            fprintf(out, "For\n");
            break;
        case NODE_RETURN:
            fprintf(out, node->child_count > 0 ? "ReturnStmt\n" : "ReturnStmt (void)\n");
            break;
        case NODE_PRINT:
            fprintf(out, "PrintStmt\n");
            break;
        case NODE_BLOCK:
            fprintf(out, "Block\n");
            break;
        case NODE_BINARY_OP:
            fprintf(out, "BinaryOp: %s\n", binop_name(node->binop));
            break;
        case NODE_UNARY_OP:
            fprintf(out, "UnaryOp: %s\n", unop_name(node->unop));
            break;
        case NODE_CALL:
            fprintf(out, "Call: %s\n", node->name);
            break;
        case NODE_LITERAL:
            switch (node->data_type) {
                case TYPE_INT:    fprintf(out, "Literal(int): %d\n", node->int_value); break;
                case TYPE_FLOAT:  fprintf(out, "Literal(float): %g\n", node->float_value); break;
                case TYPE_CHAR:   fprintf(out, "Literal(char): '%c'\n", node->char_value); break;
                case TYPE_BOOL:   fprintf(out, "Literal(bool): %s\n", node->bool_value ? "true" : "false"); break;
                case TYPE_STRING: fprintf(out, "Literal(string): \"%s\"\n", node->string_value ? node->string_value : ""); break;
                default:          fprintf(out, "Literal\n"); break;
            }
            break;
        case NODE_VAR_REF:
            fprintf(out, "VarRef: %s\n", node->name);
            break;
    }

    /* Print parameters of function decls */
    for (int i = 0; i < node->param_count; i++) {
        ast_print(node->params[i], out, indent + 2);
    }

    /* Print children */
    for (int i = 0; i < node->child_count; i++) {
        ast_print(node->children[i], out, indent + 2);
    }

    /* Print call arguments */
    for (int i = 0; i < node->arg_count; i++) {
        ast_print(node->args[i], out, indent + 2);
    }
}

/* ---------- recursive free ---------- */

void ast_free(AstNode *node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) ast_free(node->children[i]);
    for (int i = 0; i < node->arg_count; i++)   ast_free(node->args[i]);
    for (int i = 0; i < node->param_count; i++) ast_free(node->params[i]);
    free(node->children);
    free(node->args);
    free(node->params);
    free(node->name);
    free(node->string_value);
    free(node);
}
