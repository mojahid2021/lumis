# Lumis Compiler — Step-by-Step File-by-File Technical Guide

Welcome to the comprehensive, file-by-file architectural guide for the **Lumis Mini Compiler** (`.lum`). This guide provides an in-depth breakdown of every single file in the repository, explaining its purpose, internal data structures, function routines, and how data flows through the compiler pipeline.

> **For a real-time step-by-step trace of how every file does its job during a program run, see [EXECUTION_FLOW_GUIDE.md](EXECUTION_FLOW_GUIDE.md).**

---

## 1. Project Directory Architecture

The compiler follows a clean, 4-phase decoupled pipeline architecture:

```text
lumis/
├── Makefile                          # GNU Makefile build automation script
├── README.md                         # Project overview and quick start guide
├── docs/                             # Documentation suite
│   ├── GRAMMAR.md                    # Formal BNF grammar specification
│   ├── STEP_BY_STEP_FILE_GUIDE.md    # [This File] Complete file-by-file technical guide
│   └── beginner_guide/               # Step-by-step beginner educational series
│       ├── 01_COMPILER_OVERVIEW.md             # Compiler pipeline theory & concepts
│       ├── 02_PROJECT_STRUCTURE.md            # Overview of project layout & files
│       ├── 03_LEXER_AND_PARSER.md             # Flex DFA scanner & Bison LALR(1) parser
│       ├── 04_SEMANTIC_ANALYSIS_AND_SYMTAB.md # Scope stack & static type checker
│       ├── 05_CODE_GENERATION.md              # Intermediate TAC, Interpreter & C Backend
│       ├── 06_HOW_TO_RUN_AND_EXTEND.md        # CLI options, syntax guide & extensions
│       └── 07_COMPILER_DESIGN_VIVA_PREP.md    # 25 Viva defense Q&As for course exam
├── src/                              # Compiler Source Files
│   ├── main.c                        # CLI Driver & pipeline orchestrator
│   ├── ast.h / ast.c                 # Abstract Syntax Tree (AST) node definitions & printer
│   ├── symtab.h / symtab.c           # Parent-linked Scope Stack Symbol Table manager
│   ├── semantic.h / semantic.c       # Two-pass static semantic analyzer & type checker
│   ├── interp.h / interp.c           # Tree-walking in-memory AST interpreter (-r)
│   ├── lexer.l                       # Flex scanner specification (Regex -> Tokens)
│   └── parser.y                      # Bison LALR(1) parser specification (Tokens -> AST)
└── tests/                            # Test suite
    ├── valid/                        # Valid Lumis program test cases (.lum)
    └── invalid/                      # Invalid programs testing lexical/syntax/semantic errors
```

---

## 2. Complete File-by-File Breakdown

---

### `Makefile` — Build Automation Script
- **Role**: Compiles Flex and Bison specifications into C code, compiles all C source files into object files in `build/`, and links them into the `lumis` compiler binary.
- **Key Targets**:
  - `make` or `make all`: Generates `build/parser.tab.c`, `build/lex.yy.c`, compiles all `src/*.c` files, and produces `lumis`.
  - `make clean`: Removes `build/` directory and the `lumis` compiler binary.
  - `make test`: Runs `lumis` against all valid and invalid test cases in `tests/`.
- **Compiler Flags**: `-Wall -g -std=c99 -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE` for strict standards compliance.

---

### `src/main.c` — CLI Driver & Pipeline Orchestrator
- **Role**: Parses command-line arguments, opens the input Lumis source file, sets `yyin`, runs syntax and semantic validation, and executes the AST via the interpreter.
- **Supported CLI Flags**:
  - `-r`, `--run`: Executes the program in-memory using `interp_execute()`.
  - Default (no flags): Executes the program using the in-memory interpreter.

---

### `src/ast.h` & `src/ast.c` — Abstract Syntax Tree (AST) Representation
- **Role**: Defines the core data structures and functions used to represent the source program as a tree of nodes.
- **Key Enumerations**:
  - `DataType`: Supported data types (`TYPE_INT`, `TYPE_FLOAT`, `TYPE_CHAR`, `TYPE_BOOL`, `TYPE_STRING`, `TYPE_VOID`).
  - `NodeKind`: Categories of AST nodes (`NODE_PROGRAM`, `NODE_FUNC_DECL`, `NODE_VAR_DECL`, `NODE_ASSIGN`, `NODE_IF`, `NODE_WHILE`, `NODE_FOR`, `NODE_RETURN`, `NODE_PRINT`, `NODE_BLOCK`, `NODE_BINARY_OP`, `NODE_UNARY_OP`, `NODE_CALL`, `NODE_LITERAL`, `NODE_VAR_REF`).
  - `BinOp`: Binary operators (`OP_ADD`, `OP_SUB`, `OP_MUL`, `OP_DIV`, `OP_MOD`, `OP_EQ`, `OP_NEQ`, `OP_LT`, `OP_GT`, `OP_LE`, `OP_GE`, `OP_AND`, `OP_OR`).
  - `UnaryOp`: Unary operators (`OP_NOT`, `OP_NEG`).
- **Key Struct**:
  - `AstNode`: A unified tagged struct containing `kind`, source `line`, static `data_type`, `name`, literal values (`int_value`, `float_value`, `char_value`, `bool_value`, `string_value`), operator types (`binop`, `unop`), `children` dynamic array, `args` dynamic array for calls, and `params` array for function declarations.
- **Key Functions**:
  - `ast_new_*()`: Constructor routines allocating heap memory for each AST node kind.
  - `ast_add_child(parent, child)`: Dynamically resizes `parent->children` and appends a child node.
  - `ast_add_arg(call, arg)` / `ast_add_param(func, param)`: Append call arguments or function parameters.
  - `ast_print(node, out, indent)`: Pretty-prints the tree structure with indented pipe indentation.
  - `ast_free(node)`: Performs post-order depth-first traversal to safely reclaim heap memory without memory leaks.

---

### `src/symtab.h` & `src/symtab.c` — Scope-Stacked Symbol Table
- **Role**: Manages symbol declarations and implements block-nested lexical scoping using a stack of parent-linked scopes.
- **Key Data Structures**:
  - `SymKind`: Symbol classification (`SYM_VARIABLE`, `SYM_FUNCTION`, `SYM_PARAMETER`).
  - `Symbol`: Struct holding `name`, `kind`, `type`, `line`, function parameter signature (`param_count`, `param_types`), and `next` pointer in the scope's collision chain.
  - `Scope`: Struct holding a linked list of symbols (`head`), a `parent` pointer pointing to the enclosing scope, and scope debug `name`.
  - `SymbolTable`: Struct holding pointers to the `current` scope and the root `global` scope.
- **Key Functions**:
  - `symtab_init(tab)`: Initializes global scope.
  - `symtab_enter_scope(tab, name)`: Creates a new child scope pointing to `tab->current` as its parent and pushes it as the active scope.
  - `symtab_leave_scope(tab)`: Pops the current scope, safely freeing its symbol records, and restores `tab->current = old->parent`.
  - `symtab_insert(tab, name, kind, type, line)`: Inserts a new variable or parameter into the current scope after checking for duplicate local declarations.
  - `symtab_insert_func(tab, name, return_type, param_count, param_types, line)`: Registers a global function signature.
  - `symtab_lookup_current(tab, name)`: Checks only the current active scope (used for duplicate declaration checks).
  - `symtab_lookup(tab, name)`: Searches the active scope stack by walking up the `parent` pointer chain to the global scope (used for variable resolution and shadowing).
  - `symtab_print(tab, out)`: Prints the active scope stack and global symbol contents.

---

### `src/semantic.h` & `src/semantic.c` — Static Semantic Analyzer
- **Role**: Performs context-sensitive static analysis and type checking over the AST using two passes.
- **Two-Pass Algorithm**:
  - **Pass 1 (Global Function Registration)**: Scans all `NODE_FUNC_DECL` nodes at the program root and registers their signatures in the Symbol Table so function calls can be validated regardless of declaration order.
  - **Pass 2 (Body & Expression Traversal)**: Traverses function bodies and statements, enforcing static type compatibility, scope rules, and return types.
- **Modular Helper Functions**:
  - Expression Checkers: `check_expr_literal()`, `check_expr_var_ref()`, `check_expr_binary_op()`, `check_expr_unary_op()`, `check_expr_call()`.
  - Statement Checkers: `check_stmt_program()`, `check_stmt_func_decl()`, `check_stmt_var_decl()`, `check_stmt_assign()`, `check_stmt_if()`, `check_stmt_while()`, `check_stmt_for()`, `check_stmt_return()`, `check_stmt_block()`.
- **Enforced Semantic Rules**:
  1. Declaration before use.
  2. Prevention of duplicate identifier declarations in the same scope.
  3. Prohibition of `void` variable declarations (e.g., `void x;` is rejected).
  4. Type compatibility and implicit promotion (`int` $\rightarrow$ `float` promotion in assignments and operations).
  5. Function signature checking (matching argument count and parameter types).
  6. Return type verification (matching return value type against function return type; void functions cannot return values).

---

### `src/interp.h` & `src/interp.c` — Tree-Walking Interpreter Backend
- **Role**: Evaluates the AST directly in memory for zero-latency execution when invoked with `-r` / `--run` (or default).
- **Runtime Environment Structures**:
  - `Value`: Tagged union holding runtime values (`int_val`, `float_val`, `char_val`, `bool_val`, `str_val`).
  - `VarSymbol`: Linked list node binding variable names to runtime `Value` instances.
  - `Env`: Stack frame containing a list of variable bindings (`vars`) and a `parent` frame pointer.
  - `InterpState`: State container holding the AST `root`, return flag `returning`, and `return_val`.
- **Modular Helpers**:
  - Expression Evaluators: `eval_literal()`, `eval_unary_op()`, `eval_binary_op()`, `eval_call_func()`, `eval_expr()`.
  - Statement Executors: `exec_block()`, `exec_var_decl()`, `exec_assign()`, `exec_if_stmt()`, `exec_while_stmt()`, `exec_for_stmt()`, `exec_return_stmt()`, `exec_print_stmt()`, `exec_stmt()`.
- **Public Entry Point**: `interp_execute(root)` locates `main()`, initializes `global_env`, executes `main()` statement block, and returns exit code.

---

### `src/lexer.l` — Flex Scanner Specification
- **Role**: Converts input character stream into a stream of integer token IDs and populates `yylval` union members.
- **Key Token Rules**:
  - Keywords: `int`, `float`, `char`, `bool`, `string`, `void`, `if`, `else`, `while`, `for`, `return`, `print`, `true`, `false`.
  - Operators: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<=`, `>=`, `<`, `>`, `&&`, `||`, `!`, `=`.
  - Punctuation: `;`, `,`, `(`, `)`, `{`, `}`.
  - Number Literals: `{INT_NUM}` $\rightarrow$ `yylval.intval = atoi(yytext); return INT_NUM;`, `{FLOAT_NUM}` $\rightarrow$ `yylval.floatval = atof(yytext); return FLOAT_NUM;`.
  - String Literals: Parses escape sequences (`\n`, `\t`, `\r`, `\"`, `\\`) into heap-allocated strings $\rightarrow$ `yylval.strval = buf; return STRING_LIT;`.
  - Identifiers: `{ID}` $\rightarrow$ `yylval.strval = strdup(yytext); return ID;`.
  - Comments: Skips single-line `//` and block `/* ... */` comments.
- **Line Tracking**: Uses `%option yylineno` to track line numbers for diagnostic error messages.

---

### `src/parser.y` — Bison LALR(1) Syntax Parser
- **Role**: Parses token stream from Flex, checks syntax against Lumis Context-Free Grammar, resolves operator precedence, and builds the AST rooted at `ast_root`.
- **Key Declarations**:
  - `%union`: Specifies semantic types passed between Lexer and Parser (`intval`, `floatval`, `charval`, `strval`, `node`, `typeval`).
  - Precedence Directives:
    ```yacc
    %nonassoc LOWER_THAN_ELSE
    %nonassoc ELSE
    %left OR
    %left AND
    %left EQ NEQ
    %left LT GT LE GE
    %left PLUS MINUS
    %left STAR SLASH PERCENT
    %right NOT UNARY_MINUS
    ```
- **Grammar Structure**:
  - `program`: List of function declarations $\rightarrow$ sets `ast_root`.
  - `function`: Signature + parameter list + body statement block $\rightarrow$ constructs `NODE_FUNC_DECL`.
  - `stmt`: Declarations, assignments, `if`, `while`, `for`, `return`, `print`, expression statements, blocks.
  - `expr`: Operator expressions, function calls, variable references, literals $\rightarrow$ constructs expression AST nodes.
- **Error Handler**: `yyerror(s)` reports line number and near-token text when syntax rules are violated.

---

## 3. Data Flow Between Compiler Subsystems

```text
               +----------------------------------+
               |  Source Program File (.lum)      |
               +----------------------------------+
                                |
                                | (Character Stream)
                                v
               +----------------------------------+
               |  Flex Lexer (src/lexer.l)        |
               +----------------------------------+
                                |
                                | (Token Stream + yylval)
                                v
               +----------------------------------+
               |  Bison Parser (src/parser.y)     |
               +----------------------------------+
                                |
                                | (Heap-allocated AstNode Tree)
                                v
               +----------------------------------+
               |  Semantic Analyzer (semantic.c)  | <---> Symbol Table (symtab.c)
               +----------------------------------+
                                |
                                v
               +----------------------------------+
               |  Interpreter Backend             |
               |  (src/interp.c)                  |
               +----------------------------------+
                                |
                                v
                       Immediate Execution
```

---

## 4. Execution Examples

### Program Source (`tests/valid/hello.lum`)
```c
int main() {
    print("Hello, World!");
    return 0;
}
```

### Running the Program (`./lumis tests/valid/hello.lum` or `./lumis -r tests/valid/hello.lum`)
```text
=== PROGRAM EXECUTION ===
Hello, World!
=========================
Program finished with exit code 0
```
Line   4 | Token ID: 268  | Text: 'return'
Line   4 | Token ID: 272  | Text: '0'
Line   4 | Token ID: 292  | Text: ';'
Line   5 | Token ID: 297  | Text: '}'
✓ Lexical scanning completed successfully.
```
