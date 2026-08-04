# 06. How to Run, Test, and Extend Lumis

This guide walks you through building, testing, and modifying the Lumis compiler codebase.

---

## 1. Building and Running Lumis

### Prerequisites

Make sure you have GCC, Flex, Bison, and Make installed:

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install -y gcc flex bison make
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

### Running a Single File

```bash
./lumis tests/valid/hello.lum
```

---

## 2. Inspecting Compiler Outputs

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
2. **Add a `do-while` loop**: Update `parser.y`, `semantic.c`, and `codegen.c` to support `do { ... } while (cond);`.
3. **Add Constant Folding**: In `semantic.c` or `ast.c`, if an expression is `10 + 20`, simplify the AST node directly to `Literal(30)`.
