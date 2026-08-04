/* =============================================================
 *  ast.h — Abstract Syntax Tree definitions for Lumis
 * =============================================================
 *
 *  An AST is a tree representation of the source program. Each
 *  node corresponds to a syntactic construct (a function, an
 *  if-statement, an addition, ...). The parser builds the tree
 *  using the constructor functions in ast.c; later phases
 *  (semantic checker, code generator) walk it.
 *
 *  Design choices:
 *    - Every node carries a `line` field for accurate error
 *      messages even after parsing finishes.
 *    - We use a single tagged union (NodeKind) instead of a
 *      class hierarchy, which keeps the C code short and easy
 *      to read for beginners.
 * ============================================================= */

#ifndef LUMIS_AST_H
#define LUMIS_AST_H

#include <stdio.h>

/* ---------- Types ---------- */

/* The four primitive types supported by Lumis. */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_VOID    /* used internally for "no value" */
} DataType;

/* A printable name for each DataType. */
const char *type_name(DataType t);

/* ---------- AST Node Kinds ---------- */

typedef enum {
    /* declarations / top-level */
    NODE_PROGRAM,
    NODE_FUNC_DECL,

    /* statements */
    NODE_VAR_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_PRINT,
    NODE_BLOCK,

    /* expressions */
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_CALL,
    NODE_LITERAL,
    NODE_VAR_REF
} NodeKind;

/* Binary operators. */
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ,  OP_NEQ, OP_LT, OP_GT, OP_LE, OP_GE,
    OP_AND, OP_OR
} BinOp;

/* Unary operators. */
typedef enum {
    OP_NOT, OP_NEG
} UnaryOp;

/* ---------- The Node Struct ---------- */

/* We use a single struct with a tagged union so all nodes can be
   stored in the same linked list / tree without casts. */
typedef struct AstNode {
    NodeKind kind;          /* what kind of node this is */
    int line;               /* source line, for error messages */

    /* Generic children — used by statements that contain a list
       of sub-statements or sub-expressions. */
    struct AstNode **children;
    int child_count;
    int child_capacity;

    /* ---- Node-specific data ---- */
    DataType data_type;     /* for decls, expressions: the static type */

    /* identifiers (function name, variable name, ...) */
    char *name;

    /* literals */
    int  int_value;
    double float_value;
    char char_value;
    int  bool_value;

    /* operators */
    BinOp   binop;
    UnaryOp unop;

    /* function call arguments */
    struct AstNode **args;
    int arg_count;

    /* function parameter list (only on NODE_FUNC_DECL) */
    struct AstNode **params;     /* each is a NODE_VAR_DECL node */
    int param_count;
    DataType return_type;
} AstNode;

/* ---------- Constructor Functions ----------
 *
 *  Every node in the tree is created with one of these functions.
 *  Each returns a freshly heap-allocated node (free with ast_free).
 */

/* Append `child` to a node's children list. The list grows
   automatically. */
void ast_add_child(AstNode *parent, AstNode *child);

/* Set the data type of a node (most nodes use this to remember
   the declared type or the computed type of an expression). */
void ast_set_type(AstNode *n, DataType t);

/* Create each kind of node. */
AstNode *ast_new_program(int line);
AstNode *ast_new_func_decl(int line, char *name, DataType return_type);
AstNode *ast_new_var_decl(int line, char *name, DataType type);
AstNode *ast_new_assign(int line, char *name, AstNode *value);
AstNode *ast_new_if(int line, AstNode *cond, AstNode *then_branch, AstNode *else_branch);
AstNode *ast_new_while(int line, AstNode *cond, AstNode *body);
AstNode *ast_new_for(int line, AstNode *init, AstNode *cond, AstNode *update, AstNode *body);
AstNode *ast_new_return(int line, AstNode *value);   /* value may be NULL */
AstNode *ast_new_print(int line, AstNode *expr);
AstNode *ast_new_block(int line);

AstNode *ast_new_binary(int line, BinOp op, AstNode *lhs, AstNode *rhs);
AstNode *ast_new_unary(int line, UnaryOp op, AstNode *operand);
AstNode *ast_new_call(int line, char *name);
AstNode *ast_new_literal_int(int line, int value);
AstNode *ast_new_literal_float(int line, double value);
AstNode *ast_new_literal_char(int line, char value);
AstNode *ast_new_literal_bool(int line, int value);    /* 0 or 1 */
AstNode *ast_new_var_ref(int line, char *name);

/* Append an argument to a call node. */
void ast_add_arg(AstNode *call, AstNode *arg);

/* Append a parameter to a function declaration. */
void ast_add_param(AstNode *func, AstNode *param);

/* Free a tree recursively. */
void ast_free(AstNode *node);

/* Pretty-print the tree to `out`. `indent` is the current indent
   level (in spaces). */
void ast_print(AstNode *node, FILE *out, int indent);

#endif /* LUMIS_AST_H */
