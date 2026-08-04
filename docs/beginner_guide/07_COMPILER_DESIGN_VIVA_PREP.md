# 07. Compiler Design Viva & Exam Preparation Guide

This guide is specifically written to help you **defend your project** in a **Compiler Design course lab exam, presentation, or viva voce**. It covers the 25 most common questions professors ask and provides clear, confident, code-backed answers.

---

## Part 1: High-Level Architecture Questions

### Q1: What is Lumis and what toolchain is used to build it?
> **Answer**: Lumis is a 4-phase compiler for a C-like programming language. It is implemented in C using:
> - **Flex** for lexical analysis (scanning tokens using regular expressions and DFAs).
> - **Bison** for syntax analysis (LALR(1) bottom-up parser building an AST).
> - **Custom C modules** for symbol table management (`symtab.c`), type checking (`semantic.c`), TAC code generation (`codegen.c`), in-memory interpretation (`interp.c`), and native binary compilation (`c_backend.c`).
> - **GCC and Make** for build automation.

### Q2: What is the difference between a Compiler and an Interpreter? Does Lumis do both?
> **Answer**: A **compiler** translates source code into machine or intermediate code before execution. An **interpreter** executes instructions directly in-memory. 
> **Lumis does both**:
> - Default mode (`./lumis file.lum`): Acts as a **compiler front-end**, outputting TAC intermediate code.
> - Execution mode (`./lumis -r file.lum`): Acts as an **interpreter**, directly evaluating the AST in-memory.
> - Binary compilation mode (`./lumis -o bin file.lum`): Acts as a **native compiler**, producing a standalone C binary.

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

## Part 5: Code Generation, Execution & C Backend

### Q10: What is Three-Address Code (TAC)? Why is it called "Three-Address"?
> **Answer**: Three-Address Code (TAC) is an Intermediate Representation (IR) where each instruction has at most one operator and at most three address references (two operands and one result). Example: `t1 = a + b`.

### Q11: How are temporary variables and labels generated in `src/codegen.c`?
> **Answer**: Lumis uses helper functions `new_temp()` and `new_label()` that generate sequential identifiers using static counters: `t1`, `t2`, `t3`... and `L1`, `L2`, `L3`...

### Q12: How does Lumis translate an `if-else` statement to TAC?
> **Answer**:
> ```text
>     t1 = cond
>     IF_FALSE t1 GOTO L_else
>     <then_branch_instructions>
>     GOTO L_end
> LABEL L_else:
>     <else_branch_instructions>
> LABEL L_end:
> ```

### Q13: How does Lumis execute code directly in `-r` mode?
> **Answer**: The in-memory interpreter (`src/interp.c`) performs a tree-walk traversal over the AST, maintaining an environment stack of variable values and evaluating expressions recursively.

### Q14: How does Lumis compile code to a native executable binary in `-o` mode?
> **Answer**: The C backend (`src/c_backend.c`) translates the validated Lumis AST nodes into standard C code (mapping `print` to C11 `_Generic` printf wrappers), writes it to a temporary source file, and invokes `gcc` to produce a standalone executable binary.

---

## Part 6: Quick Revision Checklist for Exams

- [x] Can explain the 4 compiler phases in order.
- [x] Know where token regex rules live (`src/lexer.l`).
- [x] Know where grammar BNF rules live (`src/parser.y`).
- [x] Can explain how `AstNode` represents expressions and statements (`src/ast.h`).
- [x] Can explain how symbol scopes are linked (`src/symtab.c`).
- [x] Can demonstrate running Lumis in TAC, interpreter, and binary modes.
