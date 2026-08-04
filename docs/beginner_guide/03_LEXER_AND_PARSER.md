# 03. Lexer & Parser (Flex & Bison) Guide

This guide explains how **Flex** (Lexer) and **Bison** (Parser) work together to parse source code in Lumis.

---

## 1. Lexical Analysis with Flex (`src/lexer.l`)

The Lexer reads raw text input and produces a stream of **Tokens**.

### How Flex Works

A `.l` file has three sections separated by `%%`:

```lex
%{
/* Section 1: C Includes & Header Declarations */
#include "ast.h"
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

/* Section 2: Regular Expressions */
DIGIT       [0-9]
ID          [a-zA-Z_][a-zA-Z0-9_]*

%%
/* Section 3: Token Matching Rules & Actions */
"int"                   { return INT; }
"float"                 { return FLOAT; }
"char"                  { return CHAR; }
"bool"                  { return BOOL; }
"void"                  { return VOID; }
"if"                    { return IF; }
{DIGIT}+                { yylval.intval = atoi(yytext); return INT_NUM; }
{ID}                    { yylval.strval = strdup(yytext); return ID; }
.                       { fprintf(stderr, "Unrecognized char\n"); }
%%
```

### Passing Values to Parser (`yylval`)

When a token carries data (like integer `42` or variable name `"count"`), Flex stores the value in `yylval` before returning the token ID:

- For numbers: `yylval.intval = atoi(yytext);`
- For strings: `yylval.strval = strdup(yytext);`

---

## 2. Syntax Analysis with Bison (`src/parser.y`)

The Parser checks if the tokens follow our programming language rules (Grammar) and builds the AST.

### The `%union` Declaration

Bison uses a C `union` to allow tokens and non-terminal grammar symbols to hold different data types:

```yacc
%union {
    int intval;
    double floatval;
    char charval;
    char *strval;
    AstNode *node;
    DataType typeval;
}
```

### Operator Precedence Rules

To resolve ambiguous expressions like `1 + 2 * 3`, Bison defines precedence levels (from lowest to highest):

```yacc
%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH PERCENT
%right NOT UNARY_MINUS
```

### Grammar Rules & AST Building Actions

Each rule in Bison can run C code inside `{ ... }` when matched. `$1`, `$2`, `$3` refer to the values of the components, and `$$` is the value returned by the rule:

```yacc
assign_stmt:
    ID ASSIGN expr SEMI {
        $$ = ast_new_assign(yylineno, $1, $3);
        free($1);
    }
    ;

expr:
    expr PLUS expr {
        $$ = ast_new_binary(yylineno, OP_ADD, $1, $3);
    }
    | INT_NUM {
        $$ = ast_new_literal_int(yylineno, $1);
    }
    ;
```

---

## 3. Error Handling

- **Lexical Errors**: Caught in `lexer.l` via fallback rule `.` (unrecognized character) and unterminated block comments.
- **Syntax Errors**: Caught in `parser.y` by `yyerror(const char *s)` which prints:
  `[Syntax Error] Line <line_number>: <error_message>`
