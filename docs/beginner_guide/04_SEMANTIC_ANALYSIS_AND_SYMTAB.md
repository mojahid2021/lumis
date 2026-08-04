# 04. Semantic Analysis & Symbol Table Guide

Semantic analysis is the phase of compilation that verifies whether a syntactically correct program actually makes sense according to language rules.

---

## 1. Syntax vs. Semantics

- **Syntax** asks: *"Is the code structured correctly according to grammar rules?"*
  - Example: `x = + * 5;` $\rightarrow$ **Syntax Error** (invalid operator sequence).
- **Semantics** asks: *"Does the structured code have valid meaning?"*
  - Example: `int x = "hello";` $\rightarrow$ **Semantic Error** (assigning a string to an integer).
  - Example: `y = 10;` (where `y` was never declared) $\rightarrow$ **Semantic Error** (undeclared variable).

---

## 2. Symbol Table Architecture (`src/symtab.h`, `src/symtab.c`)

The symbol table keeps track of declared variables, functions, parameters, their data types, and their scopes.

### Data Structures

```text
SymbolTable
   │
   └── current ──► Scope ("block-1")
                      │
                      ├── head ──► Symbol ("x", SYM_VARIABLE, TYPE_INT)
                      │               │
                      │               └── next ──► Symbol ("y", SYM_VARIABLE, TYPE_FLOAT)
                      │
                      └── parent ──► Scope ("main")
                                        │
                                        ├── head ──► Symbol ("a", SYM_PARAMETER, TYPE_INT)
                                        │
                                        └── parent ──► Scope ("global")
                                                          │
                                                          └── head ──► Symbol ("main", SYM_FUNCTION, TYPE_INT)
```

### Scope Push & Pop

- **Entering a scope**: `symtab_enter_scope(table, "function_name")` pushes a new `Scope` node onto the stack.
- **Leaving a scope**: `symtab_leave_scope(table)` pops the current scope when exiting a block or function.

---

## 3. Semantic Checks Implemented (`src/semantic.c`)

1. **Declaration Before Use**: When a variable `x` is referenced, `symtab_lookup()` walks up the active scope chain. If not found $\rightarrow$ `Undeclared variable 'x'`.
2. **Redeclaration Check**: When a variable `x` is declared, `symtab_lookup_current()` checks if `x` already exists in the *current* scope. If found $\rightarrow$ `Redeclaration of variable 'x'`.
3. **Type Checking & Promotion**:
   - Arithmetic (`+`, `-`, `*`, `/`) allows `int` and `float`. Mixing `int` and `float` promotes the result to `float`.
   - Modulo `%` requires both operands to be `int`.
   - Logical operations (`&&`, `||`, `!`) require `bool`.
4. **Function Signature Validation**:
   - Ensures the number of arguments matches the declared function parameters.
   - Ensures argument types match parameter types.
5. **Return Type Checking**:
   - Compares returned expression types against function return types.
   - Enforces `void` function return rules (`return;` without expressions for `void` functions; returning expressions in `void` functions is prohibited).
6. **Void Variable Prohibition**:
   - Prevents declaring variables of type `void` (e.g., `void x;` triggers a semantic error).
