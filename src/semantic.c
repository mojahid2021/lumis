/* ============================================================================
 *  semantic.c — Static Semantic Analyzer for Lumis Compiler
 * ============================================================================
 *
 *  CSE 314 COMPILER DESIGN CONCEPTS (VIVA / DEFENSE PREPARATION):
 *
 *  1. WHAT IS SEMANTIC ANALYSIS?
 *     Semantic analysis is the third phase of compilation. While the parser
 *     verifies syntactic grammar, the semantic analyzer enforces context-sensitive
 *     rules that cannot be captured by context-free grammars (CFGs):
 *       - Scope & Declaration: All identifiers must be declared before use.
 *       - Unique Declarations: No variable or function redeclaration in the same scope.
 *       - Static Type Safety: Operands, assignments, return statements, and function
 *         arguments must have compatible static types.
 *       - Implicit Type Promotion: Int literals/expressions are implicitly promoted
 *         to float when assigned to or operated with float targets.
 *
 *  2. TWO-PASS GLOBAL SYMBOL REGISTRATION:
 *     For programs (NODE_PROGRAM), we run two passes:
 *       - Pass 1: Scan all global function declarations and insert signatures
 *         into the Symbol Table so function calls can be type-checked regardless
 *         of declaration order.
 *       - Pass 2: Recursively traverse function bodies and statements.
 * ============================================================================ */

#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* State counters for diagnostic reporting */
static int error_count = 0;
static DataType current_func_return_type = TYPE_VOID;

/* Helper diagnostic reporter */
static void report_error(int line, const char *msg) {
    fprintf(stderr, "[Semantic Error] Line %d: %s\n", line, msg);
    error_count++;
}

/* Forward declarations for recursive dispatch */
static DataType check_expression(AstNode *node, SymbolTable *symtab);
static void check_statement(AstNode *node, SymbolTable *symtab);

/* ============================================================================
 *  Expression Checker Helpers
 * ============================================================================ */

/* Node Kind: NODE_LITERAL */
static DataType check_expr_literal(AstNode *node) {
    return node->data_type;
}

/* Node Kind: NODE_VAR_REF */
static DataType check_expr_var_ref(AstNode *node, SymbolTable *symtab) {
    Symbol *sym = symtab_lookup(symtab, node->name);
    if (!sym) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Undeclared variable '%s'", node->name);
        report_error(node->line, msg);
        node->data_type = TYPE_VOID;
        return TYPE_VOID;
    }
    if (sym->kind == SYM_FUNCTION) {
        char msg[256];
        snprintf(msg, sizeof(msg), "'%s' is a function, not a variable", node->name);
        report_error(node->line, msg);
    }
    node->data_type = sym->type;
    return sym->type;
}

/* Node Kind: NODE_BINARY_OP */
static DataType check_expr_binary_op(AstNode *node, SymbolTable *symtab) {
    DataType t1 = check_expression(node->children[0], symtab);
    DataType t2 = check_expression(node->children[1], symtab);

    if (t1 == TYPE_VOID || t2 == TYPE_VOID) {
        node->data_type = TYPE_VOID;
        return TYPE_VOID;
    }

    switch (node->binop) {
        case OP_ADD:
            if (t1 == TYPE_STRING || t2 == TYPE_STRING) {
                node->data_type = TYPE_STRING;
            } else if ((t1 == TYPE_INT || t1 == TYPE_FLOAT) && (t2 == TYPE_INT || t2 == TYPE_FLOAT)) {
                node->data_type = (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            } else {
                report_error(node->line, "Addition '+' requires numeric or string types");
                node->data_type = TYPE_VOID;
            }
            break;

        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            if ((t1 == TYPE_INT || t1 == TYPE_FLOAT) && (t2 == TYPE_INT || t2 == TYPE_FLOAT)) {
                node->data_type = (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
            } else {
                report_error(node->line, "Arithmetic operations (-, *, /) require numeric types (int or float)");
                node->data_type = TYPE_VOID;
            }
            break;

        case OP_MOD:
            if (t1 == TYPE_INT && t2 == TYPE_INT) {
                node->data_type = TYPE_INT;
            } else {
                report_error(node->line, "Modulo operator '%' requires int operands");
                node->data_type = TYPE_VOID;
            }
            break;

        case OP_LT:
        case OP_GT:
        case OP_LE:
        case OP_GE:
            if ((t1 == TYPE_INT || t1 == TYPE_FLOAT || t1 == TYPE_CHAR || t1 == TYPE_STRING) &&
                (t2 == TYPE_INT || t2 == TYPE_FLOAT || t2 == TYPE_CHAR || t2 == TYPE_STRING)) {
                node->data_type = TYPE_BOOL;
            } else {
                report_error(node->line, "Relational comparisons require numeric, char, or string types");
                node->data_type = TYPE_BOOL;
            }
            break;

        case OP_EQ:
        case OP_NEQ:
            if (t1 != t2) {
                report_error(node->line, "Equality check requires matching types on left and right sides");
            }
            node->data_type = TYPE_BOOL;
            break;

        case OP_AND:
        case OP_OR:
            if (t1 == TYPE_BOOL && t2 == TYPE_BOOL) {
                node->data_type = TYPE_BOOL;
            } else {
                report_error(node->line, "Logical operations (&&, ||) require bool operands");
                node->data_type = TYPE_BOOL;
            }
            break;
    }
    return node->data_type;
}

/* Node Kind: NODE_UNARY_OP */
static DataType check_expr_unary_op(AstNode *node, SymbolTable *symtab) {
    DataType t = check_expression(node->children[0], symtab);
    if (node->unop == OP_NOT) {
        if (t != TYPE_BOOL) {
            report_error(node->line, "Logical NOT '!' requires a bool operand");
        }
        node->data_type = TYPE_BOOL;
    } else if (node->unop == OP_NEG) {
        if (t != TYPE_INT && t != TYPE_FLOAT) {
            report_error(node->line, "Unary negation '-' requires int or float");
        }
        node->data_type = t;
    }
    return node->data_type;
}

/* Node Kind: NODE_CALL */
static DataType check_expr_call(AstNode *node, SymbolTable *symtab) {
    Symbol *sym = symtab_lookup(symtab, node->name);
    if (!sym) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Call to undeclared function '%s'", node->name);
        report_error(node->line, msg);
        node->data_type = TYPE_VOID;
        return TYPE_VOID;
    }
    if (sym->kind != SYM_FUNCTION) {
        char msg[256];
        snprintf(msg, sizeof(msg), "'%s' is not a function", node->name);
        report_error(node->line, msg);
        node->data_type = TYPE_VOID;
        return TYPE_VOID;
    }

    /* Verify argument count matches parameter count */
    if (node->arg_count != sym->param_count) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Function '%s' expects %d argument(s), but %d provided",
                 node->name, sym->param_count, node->arg_count);
        report_error(node->line, msg);
    }

    /* Verify argument types match expected parameter types */
    for (int i = 0; i < node->arg_count; i++) {
        DataType arg_type = check_expression(node->args[i], symtab);
        if (i < sym->param_count && arg_type != TYPE_VOID && arg_type != sym->param_types[i]) {
            /* Allow implicit int -> float promotion */
            if (!(sym->param_types[i] == TYPE_FLOAT && arg_type == TYPE_INT)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Argument %d type mismatch in call to '%s': expected %s, got %s",
                         i + 1, node->name, type_name(sym->param_types[i]), type_name(arg_type));
                report_error(node->line, msg);
            }
        }
    }

    node->data_type = sym->type;
    return sym->type;
}

/* Master expression evaluator router */
static DataType check_expression(AstNode *node, SymbolTable *symtab) {
    if (!node) return TYPE_VOID;

    switch (node->kind) {
        case NODE_LITERAL:   return check_expr_literal(node);
        case NODE_VAR_REF:   return check_expr_var_ref(node, symtab);
        case NODE_BINARY_OP: return check_expr_binary_op(node, symtab);
        case NODE_UNARY_OP:  return check_expr_unary_op(node, symtab);
        case NODE_CALL:      return check_expr_call(node, symtab);
        default:             return TYPE_VOID;
    }
}

/* ============================================================================
 *  Statement Checker Helpers
 * ============================================================================ */

/* Node Kind: NODE_PROGRAM */
static void check_stmt_program(AstNode *node, SymbolTable *symtab) {
    /* Pass 1: Global registration of function signatures */
    for (int i = 0; i < node->child_count; i++) {
        AstNode *func = node->children[i];
        if (func->kind == NODE_FUNC_DECL) {
            DataType param_types[32];
            int pcount = func->param_count < 32 ? func->param_count : 32;
            for (int p = 0; p < pcount; p++) {
                param_types[p] = func->params[p]->data_type;
            }
            if (!symtab_insert_func(symtab, func->name, func->return_type, pcount, param_types, func->line)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Redeclaration of function '%s'", func->name);
                report_error(func->line, msg);
            }
        }
    }

    /* Pass 2: Traverse function bodies */
    for (int i = 0; i < node->child_count; i++) {
        check_statement(node->children[i], symtab);
    }
}

/* Node Kind: NODE_FUNC_DECL */
static void check_stmt_func_decl(AstNode *node, SymbolTable *symtab) {
    current_func_return_type = node->return_type;
    symtab_enter_scope(symtab, node->name);

    /* Insert function parameters into the function's local scope */
    for (int i = 0; i < node->param_count; i++) {
        AstNode *p = node->params[i];
        if (!symtab_insert(symtab, p->name, SYM_PARAMETER, p->data_type, p->line)) {
            char msg[256];
            snprintf(msg, sizeof(msg), "Redeclaration of parameter '%s'", p->name);
            report_error(p->line, msg);
        }
    }

    /* Traverse function body statement block */
    for (int i = 0; i < node->child_count; i++) {
        check_statement(node->children[i], symtab);
    }

    symtab_leave_scope(symtab);
}

/* Node Kind: NODE_VAR_DECL */
static void check_stmt_var_decl(AstNode *node, SymbolTable *symtab) {
    if (node->data_type == TYPE_VOID) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Variable '%s' cannot be declared with 'void' type", node->name);
        report_error(node->line, msg);
    }

    if (!symtab_insert(symtab, node->name, SYM_VARIABLE, node->data_type, node->line)) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Redeclaration of variable '%s' in current scope", node->name);
        report_error(node->line, msg);
    }

    if (node->child_count > 0) {
        DataType init_type = check_expression(node->children[0], symtab);
        if (init_type != TYPE_VOID && node->data_type != init_type) {
            /* Allow implicit int -> float promotion */
            if (!(node->data_type == TYPE_FLOAT && init_type == TYPE_INT)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Type mismatch in initialization of '%s': expected %s, got %s",
                         node->name, type_name(node->data_type), type_name(init_type));
                report_error(node->line, msg);
            }
        }
    }
}

/* Node Kind: NODE_ASSIGN */
static void check_stmt_assign(AstNode *node, SymbolTable *symtab) {
    Symbol *sym = symtab_lookup(symtab, node->name);
    if (!sym) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Assignment to undeclared variable '%s'", node->name);
        report_error(node->line, msg);
    } else {
        DataType rhs_type = check_expression(node->children[0], symtab);
        if (rhs_type != TYPE_VOID && sym->type != rhs_type) {
            /* Allow int -> float implicit promotion */
            if (!(sym->type == TYPE_FLOAT && rhs_type == TYPE_INT)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "Type mismatch in assignment to '%s': expected %s, got %s",
                         node->name, type_name(sym->type), type_name(rhs_type));
                report_error(node->line, msg);
            }
        }
    }
}

/* Node Kind: NODE_IF */
static void check_stmt_if(AstNode *node, SymbolTable *symtab) {
    DataType t = check_expression(node->children[0], symtab);
    if (t != TYPE_BOOL && t != TYPE_INT && t != TYPE_VOID) {
        report_error(node->line, "If condition must evaluate to boolean or integer");
    }
    check_statement(node->children[1], symtab);
    if (node->child_count > 2) {
        check_statement(node->children[2], symtab);
    }
}

/* Node Kind: NODE_WHILE */
static void check_stmt_while(AstNode *node, SymbolTable *symtab) {
    DataType t = check_expression(node->children[0], symtab);
    if (t != TYPE_BOOL && t != TYPE_INT && t != TYPE_VOID) {
        report_error(node->line, "While condition must evaluate to boolean or integer");
    }
    check_statement(node->children[1], symtab);
}

/* Node Kind: NODE_FOR */
static void check_stmt_for(AstNode *node, SymbolTable *symtab) {
    /* child[0] = init, child[1] = cond, child[2] = update, child[3] = body */
    check_statement(node->children[0], symtab);

    DataType t = check_expression(node->children[1], symtab);
    if (t != TYPE_BOOL && t != TYPE_INT && t != TYPE_VOID) {
        report_error(node->line, "For loop condition must evaluate to boolean or integer");
    }

    check_statement(node->children[2], symtab);
    check_statement(node->children[3], symtab);
}

/* Node Kind: NODE_RETURN */
static void check_stmt_return(AstNode *node, SymbolTable *symtab) {
    if (current_func_return_type == TYPE_VOID) {
        if (node->child_count > 0) {
            report_error(node->line, "Void function cannot return a value");
        }
    } else {
        if (node->child_count == 0) {
            report_error(node->line, "Non-void function must return a value");
        } else {
            DataType t = check_expression(node->children[0], symtab);
            if (t != TYPE_VOID && t != current_func_return_type) {
                if (!(current_func_return_type == TYPE_FLOAT && t == TYPE_INT)) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Return type mismatch: expected %s, got %s",
                             type_name(current_func_return_type), type_name(t));
                    report_error(node->line, msg);
                }
            }
        }
    }
}

/* Node Kind: NODE_BLOCK */
static void check_stmt_block(AstNode *node, SymbolTable *symtab) {
    symtab_enter_scope(symtab, "block");
    for (int i = 0; i < node->child_count; i++) {
        check_statement(node->children[i], symtab);
    }
    symtab_leave_scope(symtab);
}

/* Master statement router */
static void check_statement(AstNode *node, SymbolTable *symtab) {
    if (!node) return;

    switch (node->kind) {
        case NODE_PROGRAM:   check_stmt_program(node, symtab); break;
        case NODE_FUNC_DECL: check_stmt_func_decl(node, symtab); break;
        case NODE_VAR_DECL:  check_stmt_var_decl(node, symtab); break;
        case NODE_ASSIGN:    check_stmt_assign(node, symtab); break;
        case NODE_IF:        check_stmt_if(node, symtab); break;
        case NODE_WHILE:     check_stmt_while(node, symtab); break;
        case NODE_FOR:       check_stmt_for(node, symtab); break;
        case NODE_RETURN:    check_stmt_return(node, symtab); break;
        case NODE_PRINT:     check_expression(node->children[0], symtab); break;
        case NODE_BLOCK:     check_stmt_block(node, symtab); break;
        default:             check_expression(node, symtab); break;
    }
}

/* Public entry point */
int semantic_check(AstNode *root, SymbolTable *symtab) {
    error_count = 0;
    check_statement(root, symtab);
    return error_count;
}
