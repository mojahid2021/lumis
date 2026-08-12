# Lumis Compiler — Complete Step-by-Step Execution & File Guide

This document is the ultimate, student-friendly guide explaining **how every file works**, **what functions are called**, **what each component is used for**, and **how execution moves from one step to the next**. It is designed so you can explain every single detail in a lab defense, viva, or exam.

---

## 1. Quick Pipeline Overview

When you run `./lumis sample.lum`, your code flows through 6 sequential steps across the codebase:

```text
[Step 1] Setup               ➜ src/main.c
[Step 2] Read Words (Tokens) ➜ src/lexer.l
[Step 3] Build Tree (AST)    ➜ src/parser.y & src/ast.c / src/ast.h
[Step 4] Check Rules & Types ➜ src/semantic.c & src/symtab.c / src/symtab.h
[Step 5] Run Code            ➜ src/interp.c / src/interp.h
[Step 6] Free Memory & Exit  ➜ src/main.c
```

---

## 2. The Master Function Call Chain (The Code Map)

Here is the exact chain of C function calls from start to finish:

```text
main(argc, argv) [src/main.c]
 │
 ├── 1. fopen("sample.lum", "r")
 │      └── Assigns input file pointer to Flex global: `yyin = file`
 │
 ├── 2. yyparse() [src/parser.y]
 │      ├── yylex() [src/lexer.l] (Called continuously in a loop by Bison)
 │      │    ├── Reads characters from `yyin`
 │      │    ├── Matches regex patterns (keywords, identifiers, numbers)
 │      │    └── Sets `yylval` values and returns token IDs (INT, ID, INT_NUM, etc.)
 │      │
 │      └── ast_new_*() [src/ast.c] (Called when Bison reduces grammar rules)
 │           ├── ast_new_program() / ast_new_func_decl() / ast_new_var_decl()
 │           ├── ast_new_binary() / ast_new_assign() / ast_new_print()
 │           └── ast_add_child(parent, child) -> Builds tree connections
 │      └── Saves the top tree root into global variable `ast_root`
 │
 ├── 3. symtab_init(&symtab) [src/symtab.c]
 │      └── Creates the root global scope frame (`symtab.global`)
 │
 ├── 4. semantic_check(ast_root, &symtab) [src/semantic.c]
 │      └── check_statement(root, symtab) [Internal recursive dispatcher]
 │           ├── Pass 1: Registers function signatures via symtab_insert_func()
 │           └── Pass 2: Traverses function bodies and checks rules:
 │                ├── symtab_enter_scope(&symtab, "main") -> Pushes local scope
 │                ├── symtab_insert(&symtab, "x", SYM_VARIABLE, TYPE_INT, line)
 │                ├── check_expression() -> Calls symtab_lookup() to check variables
 │                └── symtab_leave_scope(&symtab) -> Pops local scope
 │
 ├── 5. interp_execute(ast_root) [src/interp.c]
 │      ├── find_function(root, "main") -> Finds main() AST node
 │      ├── env_create(NULL) -> Allocates `global_env` runtime memory frame
 │      └── exec_stmt() / eval_expr() [Tree-walking interpreter loop]
 │           ├── exec_var_decl() -> eval_expr() -> env_define(env, "x", value)
 │           ├── exec_assign()   -> eval_expr() -> env_set(env, "x", value)
 │           ├── exec_print_stmt() -> eval_expr() -> printf() to terminal screen
 │           └── exec_return_stmt() -> eval_expr() -> Sets state.returning = 1
 │      └── env_free(global_env) -> Frees runtime memory frame
 │
 └── 6. Memory Cleanup & Exit [src/main.c]
        ├── symtab_free(&symtab) [src/symtab.c] -> Frees symbol table memory
        ├── ast_free(ast_root)   [src/ast.c]   -> Frees all AST nodes recursively
        └── fclose(file)                       -> Closes file pointer
```

---

## 3. Sample Code Trace Context

We trace how the files process this sample program (`sample.lum`):

```c
int main() {
    int x = 10;
    int y = 20;
    int sum = x + y;
    print(sum);
    return 0;
}
```

---

## 4. Deep-Dive: File-by-File, Step-by-Step

---

### Step 1: Program Invocation & Setup — `src/main.c`

- **Active File**: `src/main.c`
- **What it is used for**: Acts as the main Boss / CLI Driver. It reads command-line options, opens the source file, triggers compilation, and cleans up memory at the end.
- **Key Variables Used**: `argc`, `argv`, `FILE *file`, `extern FILE *yyin`.
- **Functions Used**:
  - `main(int argc, char **argv)`: C program entry point.
  - `print_usage(prog_name)`: Displays help instructions if arguments are missing or incorrect.
  - `fopen(filename, "r")`: Opens the `.lum` file for reading.
- **Trace on `sample.lum`**:
  - `main()` receives `"sample.lum"`.
  - Opens `sample.lum` handle and assigns it to Flex scanner: `yyin = file`.
- **How it goes to Next Step**:
  `main.c` calls `yyparse()` (Step 3), which immediately triggers Flex's `yylex()` (Step 2) to start scanning characters.

---

### Step 2: Lexical Analysis — `src/lexer.l` (Flex Scanner)

- **Active File**: `src/lexer.l`
- **What it is used for**: Turns raw text into individual words called **Tokens**.
- **Key Variables Used**: `yytext` (the text matched), `yylval` (union storing token value), `yylineno` (line counter), `lexical_errors`.
- **Functions Used**:
  - `yylex()`: Scans the next token from `yyin` using regular expression matching rules.
  - `strdup(yytext)`: Copies string text into memory for identifiers (`ID`).
  - `atoi(yytext)`: Converts string numbers into integers (`INT_NUM`).
- **Trace on `sample.lum`**:
  - `"int"` $\rightarrow$ returns token ID `INT`.
  - `"main"` $\rightarrow$ sets `yylval.strval = strdup("main")`, returns `ID`.
  - `"10"` $\rightarrow$ sets `yylval.intval = 10`, returns `INT_NUM`.
  - `"+"` $\rightarrow$ returns token ID `PLUS`.
- **How it goes to Next Step**:
  `yylex()` returns token IDs back to Bison's `yyparse()` loop. Each token is shifted onto Bison's parse stack.

---

### Step 3: Syntax Analysis & AST Construction — `src/parser.y`, `src/ast.h` & `src/ast.c`

- **Active Files**: `src/parser.y` (Grammar rules), `src/ast.h` (Tree definitions), `src/ast.c` (Tree constructors).
- **What they are used for**:
  - `src/parser.y`: Checks grammar syntax rules (Context-Free Grammar) using LALR(1) parsing table.
  - `src/ast.h` & `src/ast.c`: Constructs a tree (Abstract Syntax Tree / AST) representing the code hierarchy.
- **Key Data Structures Used**:
  - `AstNode`: Tagged struct representing program nodes (statements, expressions, literals).
  - `NodeKind`: Enum specifying node type (`NODE_PROGRAM`, `NODE_FUNC_DECL`, `NODE_VAR_DECL`, `NODE_BINARY_OP`, `NODE_PRINT`, etc.).
  - `DataType`: Enum specifying variable types (`TYPE_INT`, `TYPE_FLOAT`, `TYPE_CHAR`, `TYPE_BOOL`, `TYPE_STRING`, `TYPE_VOID`).
  - `AstNode *ast_root`: Global pointer storing the root of the completed AST tree.
- **Functions Used**:
  - `yyparse()`: Bison entry point that drives the parser.
  - `ast_new_program(line)`: Allocates `NODE_PROGRAM` root node.
  - `ast_new_func_decl(line, name, type)`: Allocates function declaration node.
  - `ast_new_var_decl(line, name, type)`: Allocates variable declaration node.
  - `ast_new_binary(line, op, lhs, rhs)`: Allocates binary operation node (e.g. `x + y`).
  - `ast_add_child(parent, child)`: Dynamically resizes `children` array and connects child nodes to parent.
  - `ast_print(node, out, indent)`: Pretty-prints the AST hierarchy tree (used when `-a` flag is passed).
- **Trace on `sample.lum`**:
  - Parses `int x = 10;` $\rightarrow$ calls `ast_new_var_decl(yylineno, "x", TYPE_INT)` and attaches `ast_new_literal_int(10)`.
  - Parses `x + y` $\rightarrow$ calls `ast_new_binary(yylineno, OP_ADD, lhs, rhs)`.
  - Sets global variable `ast_root`.
- **How it goes to Next Step**:
  `yyparse()` completes and returns `0` to `main.c`. `main.c` then initializes the Symbol Table using `symtab_init(&symtab)` and calls `semantic_check(ast_root, &symtab)` (Step 4).

---

### Step 4: Static Semantic Analysis & Scope Management — `src/semantic.c`, `src/symtab.h` & `src/symtab.c`

- **Active Files**: `src/semantic.h` & `src/semantic.c` (Rule Checker), `src/symtab.h` & `src/symtab.c` (Symbol Table Notebook).
- **What they are used for**:
  - `src/symtab.c`: Manages block scopes (`{ ... }`) using a parent-linked stack of scopes to store declared variables and functions.
  - `src/semantic.c`: Verifies static language rules:
    1. Variable declared before use.
    2. No duplicate declarations in the same scope.
    3. Type checking on operations (`int + int` = `int`, `int + float` = implicit `float` promotion).
    4. Function return type verification.
- **Key Data Structures Used**:
  - `Symbol`: Struct holding variable/function properties (`name`, `kind`, `type`, `line`).
  - `Scope`: Struct holding a linked list of symbols (`head`) and pointer to parent scope (`parent`).
  - `SymbolTable`: Struct holding `current` scope and `global` scope pointers.
- **Functions Used**:
  - `symtab_init(&symtab)`: Initializes the Symbol Table and creates global scope.
  - `symtab_enter_scope(&symtab, name)`: Pushes a new nested scope onto the stack.
  - `symtab_leave_scope(&symtab)`: Pops active scope and frees local symbols.
  - `symtab_insert(&symtab, name, kind, type, line)`: Inserts a new variable into current scope (returns 0 if redeclared).
  - `symtab_insert_func(...)`: Registers global function signatures.
  - `symtab_lookup(&symtab, name)`: Searches current scope and walks up `parent` chain to find symbols.
  - `semantic_check(ast_root, &symtab)`: Public entry point that runs 2-pass analysis over the AST.
- **Trace on `sample.lum`**:
  - **Pass 1**: Registers `func main : int` into global scope.
  - **Pass 2**: Enters `main` scope $\rightarrow$ inserts `x: int`, `y: int`, `sum: int`. Checks `x + y` $\rightarrow$ verifies `x` and `y` exist via `symtab_lookup()` and verifies `TYPE_INT + TYPE_INT` produces `TYPE_INT`.
- **How it goes to Next Step**:
  `semantic_check()` returns error count. If `sem_errors == 0`, `main.c` calls `interp_execute(ast_root)` (Step 5). If errors exist, `main.c` prints diagnostic messages and halts.

---

### Step 5: Program Execution Engine — `src/interp.h` & `src/interp.c`

- **Active Files**: `src/interp.h` & `src/interp.c` (In-Memory Interpreter).
- **What it is used for**: Evaluates the AST directly in memory by walking the tree nodes and executing statements line-by-line.
- **Key Data Structures Used**:
  - `Value`: Tagged union holding runtime values (`int_val`, `float_val`, `char_val`, `bool_val`, `str_val`).
  - `VarSymbol`: Linked list node binding variable names to runtime `Value` instances.
  - `Env`: Stack frame containing a list of variable bindings (`vars`) and `parent` pointer.
  - `InterpState`: State container tracking AST `root`, return flag (`returning`), and `return_val`.
- **Functions Used**:
  - `interp_execute(ast_root)`: Main entry point that finds `main()`, creates `global_env`, and executes statements.
  - `find_function(root, name)`: Finds `NODE_FUNC_DECL` matching `"main"`.
  - `env_create(parent)`: Allocates a new runtime environment frame.
  - `env_define(env, name, val)`: Binds a new variable value in the current environment frame.
  - `env_set(env, name, val)`: Updates an existing variable value in the environment chain.
  - `env_get(env, name)`: Looks up and returns runtime `Value` for a variable name.
  - `exec_stmt(node, env, state)`: Recursive dispatcher executing statement nodes (`NODE_VAR_DECL`, `NODE_ASSIGN`, `NODE_IF`, `NODE_WHILE`, `NODE_FOR`, `NODE_PRINT`, `NODE_RETURN`).
  - `eval_expr(node, env, state)`: Recursive dispatcher evaluating expression nodes and returning `Value`.
  - `exec_print_stmt(node, env, state)`: Evaluates print expression and prints formatted value to terminal stdout via `printf()`.
- **Trace on `sample.lum`**:
  - `exec_var_decl`: Binds `"x" -> 10` and `"y" -> 20` in `env->vars`.
  - `exec_var_decl`: `eval_expr(x + y)` fetches `x` (10) and `y` (20), computes $10 + 20 = 30$, binds `"sum" -> 30`.
  - `exec_print_stmt`: Fetches `sum` (30) and calls `printf("%d\n", 30)`.
  - `exec_return_stmt`: Sets `state.returning = 1` and `return_val = 0`.
- **How it goes to Next Step**:
  `interp_execute()` frees `global_env` via `env_free()`, returns integer exit code `0` back to `main.c`.

---

### Step 6: Memory Cleanup & Program Exit — `src/main.c`

- **Active File**: `src/main.c` (assisted by `src/ast.c` & `src/symtab.c`).
- **What it is used for**: Frees all heap memory allocated during compilation and execution, and closes file handles to ensure zero memory leaks.
- **Functions Used**:
  - `symtab_free(&symtab)` [in `src/symtab.c`]: Frees all scope nodes and symbol linked lists.
  - `ast_free(ast_root)` [in `src/ast.c`]: Performs post-order depth-first traversal freeing all AST nodes, child pointers, parameter arrays, and string names.
  - `fclose(file)`: Closes input file handle.
- **Trace on `sample.lum`**:
  - Clears all heap memory.
  - Returns `exit_code` (0) to OS shell. Program finishes!

---

## 5. Summary Cheat Sheet for Lab Defense & Viva

| Step & File | Component Role | Used For | Primary Functions Called | How It Moves to Next Step |
| --- | --- | --- | --- | --- |
| **1. `src/main.c`** | CLI Driver / Boss | Opens `.lum` file & orchestrates pipeline | `fopen()`, `yyparse()` | Calls `yyparse()` to start parsing |
| **2. `src/lexer.l`** | Lexical Scanner | Converts characters to Tokens | `yylex()`, `strdup()`, `atoi()` | Returns token IDs to `yyparse()` loop |
| **3. `src/parser.y` & `src/ast.c`** | Syntax Parser & AST Builder | Verifies grammar & builds AST tree | `yyparse()`, `ast_new_*()`, `ast_add_child()` | Sets `ast_root` & returns `0` to `main()` |
| **4. `src/semantic.c` & `src/symtab.c`** | Semantic Analyzer & Symbol Table | Type checking & scope validation | `symtab_init()`, `semantic_check()`, `symtab_lookup()` | Returns error count (0 = success) to `main()` |
| **5. `src/interp.c`** | In-Memory Interpreter | Executes AST line-by-line & handles I/O | `interp_execute()`, `exec_stmt()`, `eval_expr()` | Returns exit code integer to `main()` |
| **6. `src/main.c`** | Memory Cleaner | Reclaims heap memory & closes file | `symtab_free()`, `ast_free()`, `fclose()` | Returns exit code to OS shell |


