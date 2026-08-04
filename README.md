# Lumis — A Mini Compiler

A beginner-friendly compiler for a small C-like language. Lumis walks source code through the **classic compiler pipeline**:

```text
  Source (.lum) → Lexer → Tokens → Parser → AST → Semantic Checker → Three-Address Code
```

It is implemented in **C** using **Flex** (lexer) and **Bison** (parser). The code is heavily commented and split into small modules so it is easy to read and modify.

---

## 1. Features

- **Types:** `int`, `float`, `char`, `bool`
- **Statements:** variable declarations, assignments, `if`/`else`, `while`, `for`, `return`, `print`
- **Expressions:** arithmetic, relational, logical operators with correct precedence
- **Functions:** definitions, parameters, calls, return values
- **Comments:** single-line (`//`) and block (`/* ... */`)
- **Error reporting** with line numbers at every phase
- **Output:** human-readable three-address code (TAC)

---

## 2. Prerequisites

You need the standard compiler-construction toolchain:

| Tool     | Purpose                         | Install (Debian/Ubuntu)            |
|----------|---------------------------------|------------------------------------|
| `flex`   | Lexical analyzer generator      | `sudo apt install flex`            |
| `bison`  | Parser generator                | `sudo apt install bison`           |
| `gcc`    | C compiler                      | `sudo apt install gcc`             |
| `make`   | Build automation                | `sudo apt install make`            |

Verify everything is available:

```bash
flex --version
bison --version
gcc --version
make --version
```

---

## 3. Build

From the project root:

```bash
make
```

This produces the `lumis` executable in the project root.

To run the automated test suite:

```bash
make test
```

To clean up build artifacts:

```bash
make clean
```

---

## 4. Usage

```bash
./lumis <source-file.lum>
```

Example:

```bash
./lumis tests/valid/hello.lum
```

The compiler prints, in order:

1. **Token stream** (from the lexer)
2. **Abstract Syntax Tree** (pretty-printed)
3. **Symbol table** summary
4. **Three-Address Code** (the generated "assembly-like" output)

If an error occurs at any phase, the compiler stops and reports it with the line number.

---

## 5. Example Program

`tests/valid/hello.lum`:

```c
// My first Lumis program
int main() {
    print(42);
    return 0;
}
```

Run it:

```bash
$ ./lumis tests/valid/hello.lum
=== TOKENS ===
INT     'int'              line 2
ID      'main'             line 2
LPAREN  '('                line 2
RPAREN  ')'                line 2
LBRACE  '{'                line 2
PRINT   'print'            line 3
LPAREN  '('                line 3
NUM     '42'               line 3
RPAREN  ')'                line 3
SEMI    ';'                line 3
RETURN  'return'           line 4
NUM     '0'                line 4
SEMI    ';'                line 4
RBRACE  '}'                line 5

=== AST ===
Program
  FunctionDecl: main
    ReturnStmt: 0
    PrintStmt
      Literal(int): 42

=== SYMBOL TABLE ===
Function: main

=== THREE-ADDRESS CODE ===
PRINT 42
RETURN 0
```

---

## 6. Project Structure

```text
lumis/
├── Makefile                  # Build rules
├── README.md                 # This file
├── docs/
│   ├── GRAMMAR.md            # Full language grammar
│   └── beginner_guide/       # Step-by-step guides for compiler concepts
├── src/
│   ├── main.c                # Compiler driver / entry point
│   ├── lexer.l               # Flex — tokenizer
│   ├── parser.y              # Bison — grammar + AST builder
│   ├── ast.h / ast.c         # Abstract Syntax Tree
│   ├── symtab.h / symtab.c   # Symbol table
│   ├── semantic.h / semantic.c  # Type checker
│   └── codegen.h / codegen.c    # Three-address code generator
└── tests/
    ├── valid/                # Programs that should compile
    │   ├── hello.lum
    │   ├── arithmetic.lum
    │   ├── ifelse.lum
    │   ├── whileloop.lum
    │   ├── forloop.lum
    │   └── functions.lum
    └── invalid/              # Programs that should fail each phase
        ├── lexicalError.lum
        ├── syntaxError.lum
        ├── undeclared.lum
        └── typeMismatch.lum
```

---

## 7. The Pipeline, Phase by Phase

### Phase 1 — Lexical Analysis (`lexer.l`)

Scans the source character by character and emits **tokens**. Each token has a type (keyword, identifier, number, operator…) and a line number for error reporting. Comments and whitespace are skipped.

### Phase 2 — Syntax Analysis (`parser.y`)

Takes the token stream and matches it against the **context-free grammar**. Whenever a grammar rule is reduced, a semantic action builds a node of the **Abstract Syntax Tree (AST)**. Syntax errors (missing semicolons, unmatched braces) are caught here.

### Phase 3 — Semantic Analysis (`semantic.c`)

Walks the AST and enforces the rules that grammars can't express:

- Every used variable must be declared
- No redeclaration in the same scope
- Type checking on assignments and expressions
- Function signatures match on calls and definitions

### Phase 4 — Code Generation (`codegen.c`)

Walks the validated AST and emits **three-address code** — a simple, low-level intermediate representation where each instruction has at most three operands. Example:

```text
t1 = a + b
if t1 goto L1
goto L2
LABEL L1
PRINT t1
LABEL L2
```

---

## 8. Sample Tests

Run all sample tests automatically:

```bash
make test
```

Or run manual test loops:

Valid programs (should compile successfully):

```bash
for f in tests/valid/*.lum; do echo "=== $f ==="; ./lumis "$f"; done
```

Invalid programs (should report an error):

```bash
for f in tests/invalid/*.lum; do echo "=== $f ==="; ./lumis "$f" || true; done
```

---

## 9. Troubleshooting

| Problem | Fix |
| --- | --- |
| `flex: command not found` | `sudo apt install flex` |
| `bison: command not found` | `sudo apt install bison` |
| `implicit declaration of function 'strdup'` | Ensure POSIX feature macros (`-D_POSIX_C_SOURCE=200809L`) are included — run `make clean && make` |
| `undefined reference to yylex` / `yacc` | Make sure `main.c` includes the generated headers correctly — just run `make clean && make` |
| `conflicting types for yylex` | Don't define `yylex` yourself; Flex generates it |
| Token / parse errors | Open `docs/GRAMMAR.md` to see the supported syntax |

---

## 10. Extending Lumis

Some natural next steps:

- Add arrays (a single dimension is enough — it touches every phase)
- Add a `break` / `continue` statement
- Generate real assembly for a target like x86 or a virtual machine
- Add a simple interpreter for the TAC (instead of generating code, execute it)

The code is organized so each extension usually touches one or two files at most.

---

## 11. License

This is a teaching project — feel free to copy, modify, and learn from it.
