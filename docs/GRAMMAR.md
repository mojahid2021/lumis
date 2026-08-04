# Lumis Language Grammar & Syntax Reference

This document defines the formal grammar of the **Lumis** programming language in **BNF** form, regular expressions used by the lexer, type rules, and practical `.lum` source code examples.

---

## 1. Lumis Language Overview & Syntax Guide

Lumis is a C-like compiled language. Source files use the `.lum` extension.

### Data Types
Lumis supports five primitive data types:
- `int`: Signed 32-bit integers (e.g. `42`, `-10`, `0`)
- `float`: Floating point numbers (e.g. `3.14`, `-0.5`)
- `char`: Single ASCII characters (e.g. `'a'`, `'Z'`, `'0'`)
- `bool`: Boolean values (`true` or `false`)
- `void`: Used for functions that return no value (`void greet() { ... }`)

### Variable Declarations & Assignments
Variables can be declared without initialization, or declared and initialized on a single line:

```c
// Uninitialized declarations
int count;
float temperature;
char initial;
bool isFinished;

// Initialized declarations
int age = 25;
float pi = 3.14159;
char letter = 'K';
bool isActive = true;

// Assignments
count = 10;
temperature = 98.6;
isFinished = false;
```

### Operators & Expressions
- **Arithmetic:** `+`, `-`, `*`, `/`, `%` (modulo requires `int`)
- **Relational:** `<`, `>`, `<=`, `>=`, `==`, `!=`
- **Logical:** `&&`, `||`, `!`
- **Unary:** `-` (negation), `!` (logical NOT)

Example:
```c
int a = 10;
int b = 20;
int sum = a + b * 2;          // Result: 50
bool valid = (sum > 30) && !false;
```

### Control Flow
- **If / Else Statements:**
  ```c
  if (score >= 90) {
      print(1);
  } else {
      print(0);
  }
  ```

- **While Loops:**
  ```c
  int i = 1;
  while (i <= 5) {
      print(i);
      i = i + 1;
  }
  ```

- **For Loops:**
  ```c
  for (int i = 0; i < 5; i = i + 1) {
      print(i);
  }
  ```

### Functions
Functions specify a return type, a unique identifier name, a parameter list, and a block body:

```c
int multiply(int x, int y) {
    return x * y;
}

int main() {
    int res = multiply(6, 7);
    print(res);
    return 0;
}
```

---

## 2. Lexical Tokens (Flex Regular Expressions)

| Token        | Pattern                          | Example        |
|--------------|----------------------------------|----------------|
| `INT`        | `int`                            | `int`          |
| `FLOAT`      | `float`                          | `float`        |
| `CHAR`       | `char`                           | `char`         |
| `BOOL`       | `bool`                           | `bool`         |
| `IF`         | `if`                             | `if`           |
| `ELSE`       | `else`                           | `else`         |
| `WHILE`      | `while`                          | `while`        |
| `FOR`        | `for`                            | `for`          |
| `RETURN`     | `return`                         | `return`       |
| `PRINT`      | `print`                          | `print`        |
| `TRUE`       | `true`                           | `true`         |
| `FALSE`      | `false`                          | `false`        |
| `ID`         | `[a-zA-Z_][a-zA-Z0-9_]*`         | `count`, `_x`  |
| `INT_NUM`    | `[0-9]+`                         | `42`           |
| `FLOAT_NUM`  | `[0-9]+\.[0-9]+`                 | `3.14`         |
| `CHAR_LIT`   | `'.'`                            | `'a'`          |
| `PLUS`       | `\+`                             | `+`            |
| `MINUS`      | `-`                              | `-`            |
| `STAR`       | `*`                              | `*`            |
| `SLASH`      | `/`                              | `/`            |
| `PERCENT`    | `%`                              | `%`            |
| `EQ`         | `==`                             | `==`           |
| `NEQ`        | `!=`                             | `!=`           |
| `LT`         | `<`                              | `<`            |
| `GT`         | `>`                              | `>`            |
| `LE`         | `<=`                             | `<=`           |
| `GE`         | `>=`                             | `>=`           |
| `AND`        | `&&`                             | `&&`           |
| `OR`         | `\|\|`                           | `\|\|`         |
| `NOT`        | `!`                              | `!`            |
| `ASSIGN`     | `=`                              | `=`            |
| `SEMI`       | `;`                              | `;`            |
| `COMMA`      | `,`                              | `,`            |
| `LPAREN`     | `\(`                             | `(`            |
| `RPAREN`     | `\)`                             | `)`            |
| `LBRACE`     | `\{`                             | `{`            |
| `RBRACE`     | `\}`                             | `}`            |

Comments (ignored by the lexer):

- Single-line: `// ...`
- Block: `/* ... */` (does not nest)

---

## 2. Context-Free Grammar (Bison)

```bnf
program        := function_list

function_list  := function
                | function_list function

function       := type ID LPAREN param_list RPAREN LBRACE stmt_list RBRACE
                | type ID LPAREN RPAREN          LBRACE stmt_list RBRACE

param_list     := param
                | param_list COMMA param

param          := type ID

type           := INT | FLOAT | CHAR | BOOL

stmt_list      := /* empty */
                | stmt_list stmt

stmt           := decl_stmt
                | assign_stmt
                | if_stmt
                | while_stmt
                | for_stmt
                | return_stmt
                | print_stmt
                | block

decl_stmt      := type ID SEMI
                | type ID ASSIGN expr SEMI

assign_stmt    := ID ASSIGN expr SEMI

if_stmt        := IF LPAREN expr RPAREN stmt
                | IF LPAREN expr RPAREN stmt ELSE stmt

while_stmt     := WHILE LPAREN expr RPAREN stmt

for_stmt       := FOR LPAREN decl_stmt expr SEMI assign_stmt RPAREN stmt
                | FOR LPAREN assign_stmt expr SEMI assign_stmt RPAREN stmt

return_stmt    := RETURN expr SEMI
                | RETURN SEMI

print_stmt     := PRINT LPAREN expr RPAREN SEMI

block          := LBRACE stmt_list RBRACE

expr           := logical_or_expr

logical_or_expr   := logical_and_expr
                   | logical_or_expr OR logical_and_expr

logical_and_expr  := equality_expr
                   | logical_and_expr AND equality_expr

equality_expr     := relational_expr
                   | equality_expr EQ relational_expr
                   | equality_expr NEQ relational_expr

relational_expr   := additive_expr
                   | relational_expr LT additive_expr
                   | relational_expr GT additive_expr
                   | relational_expr LE additive_expr
                   | relational_expr GE additive_expr

additive_expr     := multiplicative_expr
                   | additive_expr PLUS multiplicative_expr
                   | additive_expr MINUS multiplicative_expr

multiplicative_expr := unary_expr
                     | multiplicative_expr STAR unary_expr
                     | multiplicative_expr SLASH unary_expr
                     | multiplicative_expr PERCENT unary_expr

unary_expr        := NOT unary_expr
                   | MINUS unary_expr
                   | primary_expr

primary_expr      := ID
                   | INT_NUM
                   | FLOAT_NUM
                   | CHAR_LIT
                   | TRUE
                   | FALSE
                   | ID LPAREN arg_list RPAREN     // function call
                   | ID LPAREN RPAREN              // function call, no args
                   | LPAREN expr RPAREN

arg_list          := expr
                   | arg_list COMMA expr
```

---

## 3. Operator Precedence (highest → lowest)

1. `()` (grouping, function call)
2. `!`, unary `-`
3. `*`, `/`, `%`
4. `+`, `-`
5. `<`, `<=`, `>`, `>=`
6. `==`, `!=`
7. `&&`
8. `||`
9. `=` (assignment, right-associative)

---

## 4. Type Rules (Semantic Checker)

| Operation                  | Allowed Types                              |
|----------------------------|--------------------------------------------|
| `+ - * /`                  | `int`, `float` (with auto promotion)       |
| `%`                        | `int` only                                 |
| `< > <= >=`                | `int`, `float`, `char`                     |
| `== !=`                    | Any, but both sides must match             |
| `&& \|\| !`                | `bool` only                                |
| Assignment                 | Types must match                           |
| Function call              | Argument count and types must match        |
| `return`                   | Must match function's declared return type |

---

## 5. Scope Rules

- A program is a list of function definitions. Each function has its own scope.
- Variables declared inside a function body live until the closing `}`.
- Function parameters are local to the function.
- Two functions cannot share a name.
- A variable cannot be redeclared in the same scope.
- A variable must be declared before it is used.

---

## 6. Example Program

```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int x;
    int y;
    int result;
    x = 10;
    y = 20;
    result = add(x, y);
    print(result);
    if (result > 25) {
        print(1);
    } else {
        print(0);
    }
    return 0;
}
```
