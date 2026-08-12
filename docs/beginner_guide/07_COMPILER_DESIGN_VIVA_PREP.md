# 07. Compiler Design Viva & Exam Preparation Guide

This guide is specifically written to help you **defend your project** in a **Compiler Design course lab exam, presentation, or viva voce**. It covers the 25 most common questions professors ask and provides clear, confident, code-backed answers.

---

## Part 1: High-Level Architecture Questions

### Q1: What is Lumis and what toolchain is used to build it?
> **Answer**: Lumis is a compiler and execution engine for a C-like programming language. It is implemented in C using:
> - **Flex** for lexical analysis (scanning tokens using regular expressions and DFAs).
> - **Bison** for syntax analysis (LALR(1) bottom-up parser building an AST).
> - **Custom C modules** for symbol table management (`symtab.c`), static semantic type checking (`semantic.c`), and in-memory execution (`interp.c`).
> - **GCC and Make** for build automation.

### Q2: What is the difference between a Compiler and an Interpreter? How does Lumis work?
> **Answer**: A **compiler** analyzes source code into tokens, AST, and semantic environments; an **interpreter** executes instructions in-memory.
> **Lumis combines both**:
> - Lumis parses `.lum` source files into an Abstract Syntax Tree (AST), performs static semantic analysis and scope checking using a Symbol Table stack, and then directly evaluates the AST in-memory via its tree-walking interpreter (`./lumis file.lum` or `./lumis -r file.lum`).

> **For complete file-by-file explanations for every module, refer to [`docs/STEP_BY_STEP_FILE_GUIDE.md`](../STEP_BY_STEP_FILE_GUIDE.md).**

---

## Part 2: Lexical Analysis (Flex — `src/lexer.l`)

### Q3: How does the Lexer work under the hood? What formal model does Flex use?
> **Answer**: Flex converts user-defined regular expressions in `src/lexer.l` into a **Deterministic Finite Automaton (DFA)**. The lexer scans input character-by-character, matches longest tokens, and returns integer token IDs (e.g. `INT`, `ID`, `PLUS`) to Bison.

### Q4: How does Flex communicate token values to Bison?
> **Answer**: Flex passes token values through the global `yylval` union variable:
> - Integer literals: `yylval.intval = atoi(yytext);`
> - Identifiers: `yylval.strval = strdup(yytext);`
> Line numbers are tracked automatically using `%option yylineno`.

---

## Part 3: Syntax Analysis (Bison — `src/parser.y`)

### Q5: What parsing algorithm does Bison use? Is it top-down or bottom-up?
> **Answer**: Bison uses **LALR(1)** (Look-Ahead LR with 1 token lookahead), which is a **bottom-up parsing algorithm**. It uses shift-reduce actions to reduce input token sequences to grammar non-terminals.

### Q6: What is a Shift-Reduce conflict, and how did you resolve precedence in Lumis?
> **Answer**: A shift-reduce conflict occurs when the parser cannot decide whether to shift the next token onto the stack or reduce a rule. In Lumis, operator precedence and associativity are declared explicitly using Bison directives:
> ```yacc
> %left OR
> %left AND
> %left EQ NEQ
> %left LT GT LE GE
> %left PLUS MINUS
> %left STAR SLASH PERCENT
> %right NOT UNARY_MINUS
> ```
> For the "dangling-else" problem, `%nonassoc LOWER_THAN_ELSE` and `%nonassoc ELSE` are used.

### Q7: What is the difference between a Parse Tree and an Abstract Syntax Tree (AST)?
> **Answer**:
> - A **Parse Tree** (concrete syntax tree) contains every syntactic detail (semicolons, parentheses, braces, commas).
> - An **Abstract Syntax Tree (AST)** retains only the structural and semantic essence of the program (operator nodes, variable references, statement structures), making later phases (semantic checking and code generation) significantly faster and easier.

---

## Part 4: Semantic Analysis & Symbol Table (`src/semantic.c` & `src/symtab.c`)

### Q8: How is the Symbol Table implemented in Lumis? How does it handle nested scopes?
> **Answer**: The Symbol Table (`src/symtab.c`) is implemented as a stack of `Scope` structures linked via parent pointers (`parent`). Each `Scope` contains a linked list of `Symbol` structures (`name`, `kind`, `type`, `line`).
> - Entering a function/block: `symtab_enter_scope()` pushes a new scope node.
> - Looking up a variable: `symtab_lookup()` searches the current scope and recursively walks up parent scope pointers.
> - Exiting a scope: `symtab_leave_scope()` pops the scope node.

### Q9: What semantic checks does Lumis perform?
> **Answer**:
> 1. **Declaration before use**: Ensures referenced variables exist in scope.
> 2. **Redeclaration checks**: Prevents duplicate variable declarations in the same scope.
> 3. **Type checking & implicit promotion**: Ensures operators have valid operand types (e.g., `%` requires `int`; `+` allows `int` and `float`, auto-promoting `int` to `float`).
> 4. **Function signature validation**: Checks argument count and parameter types.
> 5. **Return type validation**: Ensures `return` expressions match function return types, and enforces `void` rules (no return value allowed for `void` functions).
> 6. **Void variable prohibition**: Prevents `void` variable declarations (e.g. `void x;` is rejected).

---

## Part 5: Execution Engine & In-Memory Interpreter

### Q10: How does Lumis execute code in `-r` mode (or default)?
> **Answer**: The in-memory interpreter (`src/interp.c`) performs a tree-walk traversal over the AST, maintaining an environment stack of variable values and evaluating expressions recursively.

---

## Part 6: Quick Revision Checklist for Exams

- [x] Can explain the compiler phases in order.
- [x] Know where token regex rules live (`src/lexer.l`).
- [x] Know where grammar BNF rules live (`src/parser.y`).
- [x] Can explain how `AstNode` represents expressions and statements (`src/ast.h`).
- [x] Can explain how symbol scopes are linked (`src/symtab.c`).
- [x] Can demonstrate running Lumis on sample programs (`./lumis <file.lum>`).
