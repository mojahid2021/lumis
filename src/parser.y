%{
/* =============================================================
 *  parser.y — Bison LALR Parser for Lumis
 * =============================================================
 *  Parses the token stream from Flex and builds an Abstract Syntax
 *  Tree (AST) rooted at `ast_root`.
 *
 *  Error handling: `yyerror` reports syntax errors with line numbers.
 * ============================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int yylex(void);
extern int yylineno;
void yyerror(const char *s);

AstNode *ast_root = NULL;
int syntax_errors = 0;
%}

/* Union for semantic values passed from Lexer */
%union {
    int intval;
    double floatval;
    char charval;
    char *strval;
    AstNode *node;
    DataType typeval;
}

/* Tokens */
%token INT FLOAT CHAR BOOL VOID
%token IF ELSE WHILE FOR RETURN PRINT
%token <intval> TRUE_LIT FALSE_LIT INT_NUM
%token <floatval> FLOAT_NUM
%token <charval> CHAR_LIT
%token <strval> ID

%token PLUS MINUS STAR SLASH PERCENT
%token EQ NEQ LT GT LE GE
%token AND OR NOT
%token ASSIGN SEMI COMMA LPAREN RPAREN LBRACE RBRACE

/* Non-terminal types */
%type <node> program function_list function param_list param
%type <node> stmt_list stmt decl_stmt assign_stmt assign_expr expr_stmt if_stmt while_stmt for_stmt return_stmt print_stmt block
%type <node> expr arg_list
%type <typeval> type

/* Operator Precedence & Associativity (Lowest to Highest) */
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right NOT UNARY_MINUS

%%

/* ---------- Grammar Rules ---------- */

program:
    function_list {
        ast_root = $1;
    }
    ;

function_list:
    function {
        $$ = ast_new_program(yylineno);
        ast_add_child($$, $1);
    }
    | function_list function {
        $$ = $1;
        ast_add_child($$, $2);
    }
    ;

function:
    type ID LPAREN param_list RPAREN LBRACE stmt_list RBRACE {
        $$ = ast_new_func_decl(yylineno, $2, $1);
        /* add parameters */
        for (int i = 0; i < $4->child_count; i++) {
            ast_add_param($$, $4->children[i]);
        }
        /* add block body */
        ast_add_child($$, $7);
        free($2);
        free($4); /* temp container node */
    }
    | type ID LPAREN RPAREN LBRACE stmt_list RBRACE {
        $$ = ast_new_func_decl(yylineno, $2, $1);
        ast_add_child($$, $6);
        free($2);
    }
    ;

param_list:
    param {
        $$ = ast_new_block(yylineno); /* helper container */
        ast_add_child($$, $1);
    }
    | param_list COMMA param {
        $$ = $1;
        ast_add_child($$, $3);
    }
    ;

param:
    type ID {
        $$ = ast_new_var_decl(yylineno, $2, $1);
        free($2);
    }
    ;

type:
    INT   { $$ = TYPE_INT; }
    | FLOAT { $$ = TYPE_FLOAT; }
    | CHAR  { $$ = TYPE_CHAR; }
    | BOOL  { $$ = TYPE_BOOL; }
    | VOID  { $$ = TYPE_VOID; }
    ;

stmt_list:
    /* empty */ {
        $$ = ast_new_block(yylineno);
    }
    | stmt_list stmt {
        $$ = $1;
        ast_add_child($$, $2);
    }
    ;

stmt:
    decl_stmt     { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | for_stmt    { $$ = $1; }
    | return_stmt { $$ = $1; }
    | print_stmt  { $$ = $1; }
    | expr_stmt   { $$ = $1; }
    | block       { $$ = $1; }
    ;

decl_stmt:
    type ID SEMI {
        $$ = ast_new_var_decl(yylineno, $2, $1);
        free($2);
    }
    | type ID ASSIGN expr SEMI {
        $$ = ast_new_var_decl(yylineno, $2, $1);
        ast_add_child($$, $4);
        free($2);
    }
    ;

assign_expr:
    ID ASSIGN expr {
        $$ = ast_new_assign(yylineno, $1, $3);
        free($1);
    }
    ;

assign_stmt:
    assign_expr SEMI {
        $$ = $1;
    }
    ;

expr_stmt:
    expr SEMI {
        $$ = $1;
    }
    ;

if_stmt:
    IF LPAREN expr RPAREN stmt %prec LOWER_THAN_ELSE {
        $$ = ast_new_if(yylineno, $3, $5, NULL);
    }
    | IF LPAREN expr RPAREN stmt ELSE stmt {
        $$ = ast_new_if(yylineno, $3, $5, $7);
    }
    ;

while_stmt:
    WHILE LPAREN expr RPAREN stmt {
        $$ = ast_new_while(yylineno, $3, $5);
    }
    ;

for_stmt:
    FOR LPAREN assign_stmt expr SEMI assign_expr RPAREN stmt {
        $$ = ast_new_for(yylineno, $3, $4, $6, $8);
    }
    | FOR LPAREN decl_stmt expr SEMI assign_expr RPAREN stmt {
        $$ = ast_new_for(yylineno, $3, $4, $6, $8);
    }
    ;

return_stmt:
    RETURN expr SEMI {
        $$ = ast_new_return(yylineno, $2);
    }
    | RETURN SEMI {
        $$ = ast_new_return(yylineno, NULL);
    }
    ;

print_stmt:
    PRINT LPAREN expr RPAREN SEMI {
        $$ = ast_new_print(yylineno, $3);
    }
    ;

block:
    LBRACE stmt_list RBRACE {
        $$ = $2;
    }
    ;

expr:
    expr PLUS expr      { $$ = ast_new_binary(yylineno, OP_ADD, $1, $3); }
    | expr MINUS expr     { $$ = ast_new_binary(yylineno, OP_SUB, $1, $3); }
    | expr STAR expr      { $$ = ast_new_binary(yylineno, OP_MUL, $1, $3); }
    | expr SLASH expr     { $$ = ast_new_binary(yylineno, OP_DIV, $1, $3); }
    | expr PERCENT expr   { $$ = ast_new_binary(yylineno, OP_MOD, $1, $3); }
    | expr EQ expr        { $$ = ast_new_binary(yylineno, OP_EQ,  $1, $3); }
    | expr NEQ expr       { $$ = ast_new_binary(yylineno, OP_NEQ, $1, $3); }
    | expr LT expr        { $$ = ast_new_binary(yylineno, OP_LT,  $1, $3); }
    | expr GT expr        { $$ = ast_new_binary(yylineno, OP_GT,  $1, $3); }
    | expr LE expr        { $$ = ast_new_binary(yylineno, OP_LE,  $1, $3); }
    | expr GE expr        { $$ = ast_new_binary(yylineno, OP_GE,  $1, $3); }
    | expr AND expr       { $$ = ast_new_binary(yylineno, OP_AND, $1, $3); }
    | expr OR expr        { $$ = ast_new_binary(yylineno, OP_OR,  $1, $3); }
    | NOT expr            { $$ = ast_new_unary(yylineno, OP_NOT, $2); }
    | MINUS expr %prec UNARY_MINUS { $$ = ast_new_unary(yylineno, OP_NEG, $2); }
    | ID                  { $$ = ast_new_var_ref(yylineno, $1); free($1); }
    | INT_NUM             { $$ = ast_new_literal_int(yylineno, $1); }
    | FLOAT_NUM           { $$ = ast_new_literal_float(yylineno, $1); }
    | CHAR_LIT            { $$ = ast_new_literal_char(yylineno, $1); }
    | TRUE_LIT            { $$ = ast_new_literal_bool(yylineno, 1); }
    | FALSE_LIT           { $$ = ast_new_literal_bool(yylineno, 0); }
    | ID LPAREN arg_list RPAREN {
        $$ = ast_new_call(yylineno, $1);
        for (int i = 0; i < $3->child_count; i++) {
            ast_add_arg($$, $3->children[i]);
        }
        free($1);
        free($3); /* temp container node */
    }
    | ID LPAREN RPAREN {
        $$ = ast_new_call(yylineno, $1);
        free($1);
    }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

arg_list:
    expr {
        $$ = ast_new_block(yylineno); /* helper container */
        ast_add_child($$, $1);
    }
    | arg_list COMMA expr {
        $$ = $1;
        ast_add_child($$, $3);
    }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "[Syntax Error] Line %d: %s\n", yylineno, s);
    syntax_errors++;
}
