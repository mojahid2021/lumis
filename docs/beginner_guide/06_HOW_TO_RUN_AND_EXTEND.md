# 06. How to Run, Test, and Extend Lumis

This guide walks you through building, testing, and modifying the Lumis compiler codebase.

---

## 1. Building and Running Lumis

### Prerequisites

Make sure you have GCC/Clang, Flex, Bison, and Make installed:

```bash
# 1. Linux (Ubuntu / Debian)
sudo apt update && sudo apt install -y gcc flex bison make

# 2. Linux (Fedora / Arch)
sudo dnf install gcc flex bison make      # Fedora
sudo pacman -S gcc flex bison make        # Arch

# 3. macOS (using Homebrew)
xcode-select --install
brew install flex bison gcc make

# macOS PATH setup for Homebrew Bison & Flex:
# Apple Silicon (M1/M2/M3/M4):
export PATH="/opt/homebrew/opt/bison/bin:/opt/homebrew/opt/flex/bin:$PATH"
# Intel Macs:
export PATH="/usr/local/opt/bison/bin:/usr/local/opt/flex/bin:$PATH"

# 4. Windows (WSL - Recommended)
wsl --install
sudo apt update && sudo apt install -y gcc flex bison make

# 5. Windows (MSYS2 / Chocolatey)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison make  # MSYS2
choco install winflexbison mingw make                                             # Chocolatey
```

### Build Commands

From the project root directory:

```bash
# Compile the Lumis executable
make

# Clean build artifacts
make clean

# Run all sample valid and invalid tests
make test
```

### Running Code

Lumis executes programs directly using its in-memory interpreter:

```bash
# Execute program directly using the in-memory interpreter
./lumis tests/valid/hello.lum

# Or explicitly pass the run flag:
./lumis -r tests/valid/hello.lum
```

---

## 2. How to Write Lumis Code (`.lum`)

Lumis uses a simple, intuitive C-like syntax. Here is how to write `.lum` programs:

### Variables & Data Types
Lumis supports five types: `int`, `float`, `char`, `bool`, and `void` (for functions).

```c
int main() {
    // Uninitialized declaration
    int count;

    // Initialized declarations
    int age = 20;
    float pi = 3.14;
    char grade = 'A';
    bool isPassed = true;

    // Assignments
    count = age + 5;
    print(count);

    return 0;
}
```

### Operators
- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `<`, `>`, `<=`, `>=`, `==`, `!=`
- Logical: `&&`, `||`, `!`

### Control Flow
```c
// If / Else
if (score >= 90) {
    print(1);
} else {
    print(0);
}

// While loop
int i = 1;
while (i <= 5) {
    print(i);
    i = i + 1;
}

// For loop
for (int j = 0; j < 5; j = j + 1) {
    print(j);
}
```

### Functions
```c
// Function returning int
int add(int a, int b) {
    return a + b;
}

// Void function returning no value
void greet() {
    print(100);
    return;
}

int main() {
    greet();
    int sum = add(10, 20);
    print(sum);
    return 0;
}
```

---

## 3. Inspecting Compiler Outputs

When you run `./lumis tests/valid/arithmetic.lum`, you can inspect each phase's output:

1. **AST Output**: Shows how expressions are grouped.
2. **Symbol Table Output**: Shows all variables registered in their respective scopes.
3. **TAC Output**: Shows the low-level intermediate code instructions.

---

## 3. Tutorial: How to Add a New Feature

Suppose you want to add a new unary operator: **increment `++`** (or a new statement type like `do-while`). Here is the step-by-step process:

### Step 1: Lexer (`src/lexer.l`)

Add token matching rule for `++`:

```lex
"++"                    { return INC; }
```

### Step 2: Parser (`src/parser.y`)

1. Declare token `%token INC` in header.
2. Add precedence rule: `%right INC`.
3. Add grammar rule to `expr`:

   ```yacc
   expr:
       ID INC {
           /* Create AST node for x = x + 1 */
           AstNode *var = ast_new_var_ref(yylineno, $1);
           AstNode *one = ast_new_literal_int(yylineno, 1);
           AstNode *add = ast_new_binary(yylineno, OP_ADD, var, one);
           $$ = ast_new_assign(yylineno, $1, add);
           free($1);
       }
       ;
   ```

### Step 3: Rebuild and Test

```bash
make clean && make
```

Create a file `test_inc.lum` with `x++;` and run `./lumis test_inc.lum`!

---

## 4. Beginner Exercises

Try completing these exercises to deepen your compiler design skills:

1. **Add String Literals**: Update `lexer.l` and `parser.y` to support string literals like `"Hello World"`.
2. **Add a `do-while` loop**: Update `parser.y`, `semantic.c`, and `interp.c` to support `do { ... } while (cond);`.
3. **Add Constant Folding**: In `semantic.c` or `ast.c`, if an expression is `10 + 20`, simplify the AST node directly to `Literal(30)`.
