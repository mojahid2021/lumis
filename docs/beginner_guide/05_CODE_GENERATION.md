# 05. Code Generation (Three-Address Code / TAC) Guide

Code Generation is the final phase of the Lumis compilation pipeline. It traverses the validated Abstract Syntax Tree (AST) and emits **Three-Address Code (TAC)**.

---

## 1. What is Three-Address Code (TAC)?

Three-Address Code is an **Intermediate Representation (IR)** where every instruction has at most **one operator** and at most **three operands**.

### Why use TAC?

- Real assembly languages (like x86, ARM, or RISC-V) vary greatly between processors.
- TAC acts as a universal, machine-independent step that is easy to understand, optimize, or translate into target machine code.

---

## 2. TAC Instruction Format in Lumis

| Category | High-Level Code | Lumis TAC Output |
| --- | --- | --- |
| Arithmetic | `c = a + b * 2;` | `t1 = b * 2`, `t2 = a + t1`, `c = t2` |
| Assignment | `x = 10;` | `x = 10` |
| Print | `print(x);` | `PRINT x` |
| Return | `return 0;` | `RETURN 0` |
| Function Decl | `int main() { ... }` | `FUNC main: ... END FUNC` |
| Function Call | `res = add(x, y);` | `PARAM x`, `PARAM y`, `t1 = CALL add, 2`, `res = t1` |

---

## 3. Translating Control Structures

### `if / else` Translation

High-level code:

```c
if (x > 10) {
    print(1);
} else {
    print(0);
}
```

Generated TAC:

```text
    t1 = x > 10
    IF_FALSE t1 GOTO L1
    PRINT 1
    GOTO L2
LABEL L1:
    PRINT 0
LABEL L2:
```

### `while` Loop Translation

High-level code:

```c
while (i < 5) {
    i = i + 1;
}
```

Generated TAC:

```text
LABEL L1:
    t1 = i < 5
    IF_FALSE t1 GOTO L2
    t2 = i + 1
    i = t2
    GOTO L1
LABEL L2:
```

---

## 4. Implementation Details (`src/codegen.c`)

- `new_temp()`: Generates unique temporary variable names (`t1`, `t2`, `t3`, ...).
- `new_label()`: Generates unique jump labels (`L1`, `L2`, `L3`, ...).
- `gen_expr(AstNode *node, FILE *out)`: Emits TAC for expressions and returns the temporary variable or constant holding the result.
- `gen_stmt(AstNode *node, FILE *out)`: Recursively walks statement nodes in the AST and outputs structured TAC instructions.
