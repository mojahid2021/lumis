# Lumis Compiler — Step-by-Step Execution Flow & File Breakdown

This document provides a detailed, step-by-step walkthrough of how **every single file in `src/`** performs its role during the execution of a Lumis program (`.lum`).

---

## 1. Executive Summary: The 6-Step Execution Timeline

When you run `./lumis tests/valid/hello.lum` in your terminal, the execution moves sequentially through 6 major stages across the codebase:

```text
1. CLI Invocation       ➜ src/main.c
2. Lexical Tokenizing    ➜ src/lexer.l
3. Syntax Parsing & AST  ➜ src/parser.y & src/ast.c / src/ast.h
4. Semantic Analysis    ➜ src/semantic.c & src/symtab.c / src/symtab.h
5. Interpreter Execution ➜ src/interp.c / src/interp.h
6. Memory Cleanup       ➜ src/main.c (ast_free, symtab_free)
```

---

## 2. Sample Code Context for Walkthrough

To see how every file behaves, consider this sample Lumis program (`sample.lum`):

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

## 3. Detailed Step-by-Step File-by-File Job Description

### Step 1: Program Invocation & Setup — `src/main.c`

- **Role**: The CLI entry point and pipeline orchestrator.
- **Detailed Execution Steps**:
  1. `main(argc, argv)` receives command-line arguments.
  2. Parses flags (`-r` or default execution mode) and captures input filename (`sample.lum`).
  3. Opens `sample.lum` for reading via `fopen()`.
  4. Assigns the open file stream handle `file` to Flex's global input pointer `yyin = file`.
  5. Initiates the parsing phase by invoking `yyparse()`.

---

### Step 2: Lexical Analysis — `src/lexer.l` (Flex Scanner)

- **Role**: Converts raw character stream from `yyin` into integer token IDs and semantic token values (`yylval`).
- **Detailed Execution Steps**:
  1. Flex reads characters sequentially from `yyin`.
  2. Matches regular expressions defined in `src/lexer.l`:
     - `"int"` $\rightarrow$ returns token ID `INT`.
     - `"main"` $\rightarrow$ matches `{ID}` regex $\rightarrow$ allocates `yylval.strval = strdup("main")` and returns token ID `ID`.
     - `"10"` $\rightarrow$ matches `{INT_NUM}` regex $\rightarrow$ sets `yylval.intval = 10` and returns token ID `INT_NUM`.
     - `"+"` $\rightarrow$ returns token ID `PLUS`.
     - `"print"` $\rightarrow$ returns token ID `PRINT`.
  3. Skips spaces, tabs, and comments (`//` and `/* ... */`).
  4. Tracks source line numbers using `%option yylineno` for line-numbered error diagnostics.
  5. Returns each matched token ID to Bison's `yyparse()` loop.

---

### Step 3: Syntax Analysis & AST Construction — `src/parser.y`, `src/ast.h` & `src/ast.c`

- **Role**: Validates syntax against context-free grammar rules, resolves operator precedence, and builds the Abstract Syntax Tree (AST).
- **Detailed Execution Steps**:
  1. `src/parser.y` receives tokens from `yylex()`.
  2. Uses LALR(1) shift-reduce actions to verify grammar syntax.
  3. When a grammar rule is reduced, Bison executes associated C semantic actions that call constructor functions in `src/ast.c`:
     - Parsing `int x = 10;` $\rightarrow$ calls `ast_new_var_decl(yylineno, "x", TYPE_INT)` and attaches expression child `ast_new_literal_int(10)`.
     - Parsing `x + y` $\rightarrow$ calls `ast_new_binary(yylineno, OP_ADD, lhs, rhs)`.
     - Parsing `print(sum);` $\rightarrow$ calls `ast_new_print(yylineno, expr)`.
     - Parsing block `{ ... }` $\rightarrow$ calls `ast_new_block(yylineno)` and uses `ast_add_child()` to append statements.
  4. Once the top-level `program` grammar rule completes, `parser.y` sets the global AST root pointer `ast_root`.

---

### Step 4: Static Semantic Analysis & Scope Management — `src/semantic.c`, `src/symtab.h` & `src/symtab.c`

- **Role**: Enforces static language rules (declaration before use, scope resolution, type compatibility, return matching).
- **Detailed Execution Steps**:
  1. `main.c` initializes the Symbol Table data structure via `symtab_init(&symtab)` in `src/symtab.c`.
  2. `main.c` calls `semantic_check(ast_root, &symtab)` in `src/semantic.c`.
  3. **Pass 1 (Global Signature Registration)**:
     - Scans `ast_root` for all `NODE_FUNC_DECL` nodes.
     - Calls `symtab_insert_func()` to register function signatures (`main : TYPE_INT`) into `symtab.global` scope.
  4. **Pass 2 (Body Traversal & Type Checking)**:
     - Enters function body $\rightarrow$ calls `symtab_enter_scope(&symtab, "main")` to push a new scope frame.
     - Encountering `NODE_VAR_DECL` (`int x = 10;`) $\rightarrow$ calls `symtab_insert()` to insert `x : TYPE_INT` into active scope. Rejects duplicate local declarations.
     - Encountering `NODE_VAR_REF` (`x`) $\rightarrow$ calls `symtab_lookup()` to search the scope stack starting from `current` and climbing up `parent` pointers. Binds `data_type = TYPE_INT` to AST node.
     - Encountering `NODE_BINARY_OP` (`x + y`) $\rightarrow$ verifies operand types (`TYPE_INT + TYPE_INT`), performs implicit type promotion if mixed with `float`, and sets expression node type to `TYPE_INT`.
     - Exiting block $\rightarrow$ calls `symtab_leave_scope(&symtab)` to pop active scope frame.
  5. Returns total semantic error count. If errors $> 0$, `main.c` prints error diagnostic messages and halts execution.

---

### Step 5: Program Execution Engine — `src/interp.h` & `src/interp.c`

- **Role**: Evaluates the validated AST in-memory using recursive tree-walking and dynamic runtime environment frames.
- **Detailed Execution Steps**:
  1. `main.c` calls `interp_execute(ast_root)`.
  2. `interp_execute()` searches `ast_root` children to find `NODE_FUNC_DECL` for `"main"`.
  3. Allocates top-level runtime environment frame `Env global_env`.
  4. Calls `exec_stmt()` to execute `main`'s block statement list recursively:
     - **Executing `int x = 10;`**: Calls `eval_expr()` to evaluate literal `10`, then binds `"x" -> Value(int: 10)` in `env->vars`.
     - **Executing `int y = 20;`**: Evaluates `20` and binds `"y" -> Value(int: 20)`.
     - **Executing `int sum = x + y;`**: `eval_expr(x + y)` looks up `"x"` (10) and `"y"` (20) in `env->vars`, computes $10 + 20 = 30$, and binds `"sum" -> Value(int: 30)`.
     - **Executing `print(sum);`**: Evaluates expression `sum` (30) and outputs `30` to `stdout`.
     - **Executing `return 0;`**: Evaluates expression `0`, sets `InterpState.returning = 1` and `return_val = Value(int: 0)`.
  5. Reclaims runtime environment memory frames and returns exit code `0`.

---

## 4. Architectural Summary Table

| Execution Phase | Primary Source Files | Key Data Structures | Key Functions / APIs | Output Artifact |
| --- | --- | --- | --- | --- |
| **1. Driver & File Setup** | `src/main.c` | `FILE *file`, `yyin` | `fopen()`, `yyparse()` | Open file handle |
| **2. Lexical Tokenizing** | `src/lexer.l` | Token IDs, `yylval` union | `yylex()`, `strdup()` | Token stream |
| **3. Syntax Parsing & AST** | `src/parser.y`, `src/ast.c`, `src/ast.h` | `AstNode`, `NodeKind`, `DataType` | `ast_new_*()`, `ast_add_child()` | `AstNode *ast_root` |
| **4. Semantic Checking** | `src/semantic.c`, `src/symtab.c`, `src/symtab.h` | `SymbolTable`, `Scope`, `Symbol` | `symtab_insert()`, `symtab_lookup()`, `semantic_check()` | Type-validated AST |
| **5. Program Execution** | `src/interp.c`, `src/interp.h` | `Env`, `VarSymbol`, `Value` | `interp_execute()`, `eval_expr()`, `exec_stmt()` | Console I/O & exit code |
| **6. Heap Deallocation** | `src/main.c`, `src/ast.c`, `src/symtab.c` | Heap pointers | `ast_free()`, `symtab_free()`, `fclose()` | Reclaimed memory |
