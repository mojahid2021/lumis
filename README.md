# Lumis — A Mini Compiler
<p align="center">
  <img src="assets/lumis.png" alt="Lumis Compiler Logo" width="100%" />
</p>

<p align="center">
  <b>A Educational Compiler &amp; Execution Engine for Compiler Design Course</b><br>
  Built in C using Flex (Lexer) and Bison (Parser)
</p>

A beginner-friendly compiler for a small C-like language. Lumis walks source code through the **classic compiler pipeline**:

```text
  Source (.lum) → Lexer → Tokens → Parser → AST → Semantic Checker → Three-Address Code
```

It is implemented in **C** using **Flex** (lexer) and **Bison** (parser). The code is heavily commented and split into small modules so it is easy to read and modify.

---

## 1. Features

- **Types:** `int`, `float`, `char`, `bool`, `void`
- **Statements:** variable declarations (initialized/uninitialized), assignments, `if`/`else`, `while`, `for`, `return`, `print`
- **Expressions:** arithmetic (`+ - * / %`), relational (`< > <= >= == !=`), logical (`&& || !`), unary operators with standard precedence
- **Functions:** definitions, parameters, function calls, `void` and typed return values
- **Comments:** single-line (`//`) and block (`/* ... */`)
- **Error reporting:** descriptive line-numbered lexical, syntax, and semantic error diagnostics
- **Multiple Back-End Modes:**
  1. **Three-Address Code (TAC):** Machine-independent intermediate code generator
  2. **In-Memory AST Interpreter (`-r`):** Direct statement execution engine
  3. **Native Binary Compiler (`-o`):** C code generator & GCC compiler driver

---

## 2. Prerequisites & Platform Setup

Lumis requires the standard C compiler-construction toolchain (`flex`, `bison`, `gcc`/`clang`, `make`).

### OS Installation Matrix

| OS Platform | Installation Command |
|-------------|----------------------|
| **Linux (Ubuntu / Debian)** | `sudo apt update && sudo apt install -y gcc flex bison make` |
| **Linux (Fedora)** | `sudo dnf install gcc flex bison make` |
| **Linux (Arch Linux)** | `sudo pacman -S gcc flex bison make` |
| **macOS (Homebrew)** | `xcode-select --install` <br> `brew install flex bison gcc make` |
| **Windows (WSL - Recommended)** | `wsl --install` <br> `sudo apt update && sudo apt install -y gcc flex bison make` |
| **Windows (MSYS2 / MinGW)** | `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison make` |
| **Windows (Chocolatey)** | `choco install winflexbison mingw make` |

### macOS PATH Setup (Homebrew)
On macOS, Homebrew installs `bison` and `flex` in non-standard keg paths. Export PATH before building:

```bash
# Apple Silicon Macs (M1/M2/M3/M4):
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"

# Intel Macs:
export PATH="/usr/local/opt/bison/bin:/usr/local/opt/flex/bin:$PATH"
```

### Windows Setup Notes
- **WSL (Recommended)**: Open WSL terminal (Ubuntu) and run Linux commands. Lumis produces native `./lumis` binaries directly.
- **MSYS2 / Git Bash / CMD**: Make sure `flex`, `bison`, `gcc`, and `make` are added to your Windows `%PATH%`. Running `make` generates `lumis.exe`.

### Verification Command

```bash
flex --version
bison --version
gcc --version || clang --version
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

## 4. Usage & Diagnostic Modes

Lumis supports multi-mode execution flags for inspecting compiler passes and running programs:

```bash
# 1. Default Diagnostic Pipeline (AST, Symbol Table, TAC)
./lumis tests/valid/hello.lum

# 2. Dump Lexical Scanned Tokens (-t, --tokens)
./lumis -t tests/valid/hello.lum

# 3. Pretty-print Abstract Syntax Tree (-p, --ast)
./lumis -p tests/valid/hello.lum

# 4. Semantic Checks & Symbol Table Dump (-s, --symtab)
./lumis -s tests/valid/hello.lum

# 5. Output Intermediate Three-Address Code (-c, --tac)
./lumis -c tests/valid/hello.lum

# 6. Execute Program In-Memory (-r, --run)
./lumis -r tests/valid/hello.lum

# 7. Compile Program to Standalone Native Binary via GCC (-o)
./lumis -o hello tests/valid/hello.lum
./hello
```

If an error occurs at any phase, the compiler stops and reports it with line-numbered diagnostics.

---

## 5. Documentation Suite

- [**Step-by-Step File-by-File Technical Guide**](docs/STEP_BY_STEP_FILE_GUIDE.md): Deep-dive into every function, struct, and module in the repository.
- [**Grammar Specification**](docs/GRAMMAR.md): Complete BNF syntax rules.
- [**Beginner Educational Guide Series**](docs/beginner_guide/):
  - [01. Compiler Overview & Pipeline](docs/beginner_guide/01_COMPILER_OVERVIEW.md)
  - [02. Project Structure & Files](docs/beginner_guide/02_PROJECT_STRUCTURE.md)
  - [03. Lexer & Parser (Flex & Bison)](docs/beginner_guide/03_LEXER_AND_PARSER.md)
  - [04. Semantic Analysis & Symbol Table](docs/beginner_guide/04_SEMANTIC_ANALYSIS_AND_SYMTAB.md)
  - [05. Code Generation & Execution Engine](docs/beginner_guide/05_CODE_GENERATION.md)
  - [06. How to Run, Test, and Extend](docs/beginner_guide/06_HOW_TO_RUN_AND_EXTEND.md)
  - [07. Compiler Design Viva & Exam Prep](docs/beginner_guide/07_COMPILER_DESIGN_VIVA_PREP.md)

---

## 6. Lumis Language Syntax Reference (`.lum` Code)

Writing `.lum` programs is fast and intuitive:

### Declaring & Using Variables

Variables can be declared with or without initial values:

```c
int main() {
    // 1. Uninitialized variable declarations
    int count;
    float total;

    // 2. Initialized variable declarations
    int age = 20;
    float pi = 3.14159;
    char letter = 'A';
    bool isReady = true;

    // 3. Assignments
    count = age + 10;
    total = pi * 2.0;

    // 4. Output
    print(count);      // Prints: 30
    print(isReady);    // Prints: true

    return 0;
}
```

### Control Flow (If / Else, Loops)

```c
int main() {
    // If / Else
    int score = 85;
    if (score >= 90) {
        print(1);
    } else {
        print(0);
    }

    // While loop
    int i = 1;
    while (i <= 3) {
        print(i);
        i = i + 1;
    }

    // For loop
    for (int j = 0; j < 3; j = j + 1) {
        print(j);
    }

    return 0;
}
```

### Functions

```c
int square(int n) {
    return n * n;
}

int main() {
    int val = square(5);
    print(val); // 25
    return 0;
}
```

---

## 6. Example Program Pipeline Output

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

## 7. Project Structure & Beginner Guides

```text
lumis/
├── Makefile                  # Build rules
├── README.md                 # Project documentation & reference
├── Mini Compiler (Lumis).md  # Project specification & proposal
├── docs/
│   ├── GRAMMAR.md            # Full language grammar & syntax reference
│   └── beginner_guide/       # Step-by-step guides for Compiler Design course
│       ├── 01_COMPILER_OVERVIEW.md         # Theory, architecture & pipeline
│       ├── 02_PROJECT_STRUCTURE.md         # File-by-file breakdown & data flow
│       ├── 03_LEXER_AND_PARSER.md          # Flex & Bison deep dive
│       ├── 04_SEMANTIC_ANALYSIS_AND_SYMTAB.md # Scope & type checking
│       ├── 05_CODE_GENERATION.md          # TAC, Interpreter & C Backend
│       ├── 06_HOW_TO_RUN_AND_EXTEND.md     # How to run, test & add features
│       └── 07_COMPILER_DESIGN_VIVA_PREP.md # 25+ Exam defense & viva Q&As
├── src/
│   ├── main.c                # Driver entry point & CLI options
│   ├── lexer.l               # Flex — scanner (DFAs & Regex)
│   ├── parser.y              # Bison — LALR(1) parser & AST builder
│   ├── ast.h / ast.c         # Abstract Syntax Tree structures
│   ├── symtab.h / symtab.c   # Symbol table & scope management
│   ├── semantic.h / semantic.c  # Semantic analyzer & type checker
│   ├── codegen.h / codegen.c    # Three-address code generator (TAC)
│   ├── interp.h / interp.c       # AST Interpreter / Execution Engine (-r)
│   └── c_backend.h / c_backend.c # C Backend & GCC Binary Compiler (-o)
└── tests/
    ├── valid/                # Valid sample programs (.lum)
    │   ├── variables.lum     # Comprehensive syntax reference
    │   ├── void_func.lum     # Void function reference
    │   ├── hello.lum
    │   ├── arithmetic.lum
    │   ├── ifelse.lum
    │   ├── whileloop.lum
    │   ├── forloop.lum
    │   └── functions.lum
    └── invalid/              # Deliberately invalid programs (.lum)
        ├── lexicalError.lum
        ├── syntaxError.lum
        ├── undeclared.lum
        └── typeMismatch.lum
```

---

## 8. The Pipeline, Phase by Phase

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

## 9. Sample Tests

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

## 10. Troubleshooting

| Problem | Fix |
| --- | --- |
| `flex: command not found` | **Linux**: `sudo apt install flex` <br> **macOS**: `brew install flex` & export PATH <br> **Windows**: Install MSYS2 or `winflexbison` via Chocolatey |
| `bison: command not found` | **Linux**: `sudo apt install bison` <br> **macOS**: `brew install bison` & export PATH <br> **Windows**: Install MSYS2 or `winflexbison` via Chocolatey |
| macOS outdated Bison error | Run `export PATH="/opt/homebrew/opt/bison/bin:$PATH"` |
| Windows `win_flex` / `win_bison` | Rename or alias `win_flex.exe` to `flex.exe` and `win_bison.exe` to `bison.exe` or use WSL |
| `implicit declaration of function 'strdup'` | Ensure POSIX feature macros (`-D_POSIX_C_SOURCE=200809L`) are included — run `make clean && make` |
| `undefined reference to yylex` / `yacc` | Make sure `main.c` includes generated headers — run `make clean && make` |
| `conflicting types for yylex` | Don't define `yylex` yourself; Flex generates it |
| Token / parse errors | Open `docs/GRAMMAR.md` to see the supported syntax |

---

## 11. Extending Lumis

Some natural next steps:

- Add arrays (a single dimension is enough — it touches every phase)
- Add a `break` / `continue` statement
- Generate real assembly for a target like x86 or a virtual machine
- Add a simple interpreter for the TAC (instead of generating code, execute it)

The code is organized so each extension usually touches one or two files at most.

---

## 12. License

This is a teaching project — feel free to copy, modify, and learn from it.
