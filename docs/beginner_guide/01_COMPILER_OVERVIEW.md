# 01. Lumis Compiler — Theoretical Overview & Workflow

Welcome to Lumis! This guide explains how a compiler works from scratch in a beginner-friendly way.

---

## 1. What is a Compiler?

A **compiler** is a program that reads source code written in a human-readable language (like C, Java, or Lumis `.lum` files) and translates it into a lower-level language (like assembly, machine code, or Three-Address Code), while verifying that the code is free of errors.

---

## 2. The 4-Phase Pipeline Architecture

Lumis is structured into 4 distinct phases:

```text
               +----------------------------------+
               |        Source File (.lum)        |
               +----------------------------------+
                                |
                                v
               +----------------------------------+
               |  Phase 1: Lexical Analysis       |  Flex (lexer.l)
               |  (Converts text into Tokens)     |
               +----------------------------------+
                                |  Token Stream
                                v
               +----------------------------------+
               |  Phase 2: Syntax Analysis        |  Bison (parser.y)
               |  (Constructs AST Tree)           |  (ast.h / ast.c)
               +----------------------------------+
                                |  AST Tree
                                v
               +----------------------------------+
               |  Phase 3: Semantic Analysis      |  (semantic.c)
               |  (Type & Scope Validation)       |  (symtab.c)
               +----------------------------------+
                                |  Validated AST
                                v
               +----------------------------------+
               |  Phase 4: Code Generation        |  (codegen.c)
               |  (Emits Three-Address Code / TAC) |
               +----------------------------------+
```

---

## 3. Step-by-Step Example

Let's trace what happens when Lumis compiles this simple program:

```c
int main() {
    int x;
    x = 10 + 20;
    print(x);
    return 0;
}
```

### Phase 1: Lexical Analysis (Scanning)

The **Lexer** (`lexer.l`) reads characters and groups them into meaningful chunks called **Tokens**:

- `int` $\rightarrow$ `INT` keyword
- `main` $\rightarrow$ `ID("main")`
- `(` $\rightarrow$ `LPAREN`
- `)` $\rightarrow$ `RPAREN`
- `{` $\rightarrow$ `LBRACE`
- `x = 10 + 20;` $\rightarrow$ `ID("x")`, `ASSIGN`, `INT_NUM(10)`, `PLUS`, `INT_NUM(20)`, `SEMI`

### Phase 2: Syntax Analysis (Parsing)

The **Parser** (`parser.y`) checks if tokens match the rules of our programming grammar. As rules match, it builds an **Abstract Syntax Tree (AST)**:

```text
Program
  FunctionDecl: main -> int
    Block
      VarDecl: x : int
      Assign: x
        BinaryOp: +
          Literal(int): 10
          Literal(int): 20
      PrintStmt
        VarRef: x
      ReturnStmt
        Literal(int): 0
```

### Phase 3: Semantic Analysis (Meaning & Rules)

The **Semantic Analyzer** (`semantic.c`) traverses the AST:

1. Registers `main` and `x` into the **Symbol Table**.
2. Checks that `x` is declared before being assigned.
3. Checks that `10` (int) + `20` (int) yields `int`, matching `x` (int).

### Phase 4: Code Generation (TAC)

The **Code Generator** (`codegen.c`) walks the AST and emits **Three-Address Code (TAC)**:

```text
FUNC main:
    t1 = 10 + 20
    x = t1
    PRINT x
    RETURN 0
    END FUNC
```

---

## Next Steps

- Read [02_PROJECT_STRUCTURE.md](./02_PROJECT_STRUCTURE.md) to understand which file handles what!
