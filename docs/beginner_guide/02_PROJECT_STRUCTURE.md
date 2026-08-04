# 02. Project Structure & File Guide

This document breaks down every file in the `lumis` repository, explaining its purpose, contents, and role in the compilation process.

---

## Directory Map

```text
lumis/
├── Makefile                      # Automated build script
├── README.md                     # High-level project documentation
├── Mini Compiler (Lumis).md      # Original proposal and specifications
├── docs/
│   ├── GRAMMAR.md                # Language grammar rules (BNF format)
│   └── beginner_guide/           # Detailed educational guides
│       ├── 01_COMPILER_OVERVIEW.md
│       ├── 02_PROJECT_STRUCTURE.md
│       ├── 03_LEXER_AND_PARSER.md
│       ├── 04_SEMANTIC_ANALYSIS_AND_SYMTAB.md
│       ├── 05_CODE_GENERATION.md
│       ├── 06_HOW_TO_RUN_AND_EXTEND.md
│       └── 07_COMPILER_DESIGN_VIVA_PREP.md  # Viva & Exam defense guide
├── src/                          # Compiler Source Code (C, Flex, Bison)
│   ├── main.c                    # Driver entry point & CLI options
│   ├── lexer.l                   # Flex scanner specification
│   ├── parser.y                  # Bison parser specification
│   ├── ast.h / ast.c             # Abstract Syntax Tree representation
│   ├── symtab.h / symtab.c       # Symbol Table & scope management
│   ├── semantic.h / semantic.c   # Type checking & semantic analyzer
│   ├── codegen.h / codegen.c     # TAC code generator
│   ├── interp.h / interp.c       # AST Interpreter / Execution Engine (-r)
│   └── c_backend.h / c_backend.c # C Backend & GCC Binary Compiler (-o)
└── tests/                        # Test suite
    ├── valid/                    # Valid programs (.lum)
    │   ├── variables.lum         # Complete reference for variables & syntax
    │   ├── hello.lum
    │   ├── arithmetic.lum
    │   ├── ifelse.lum
    │   ├── whileloop.lum
    │   ├── forloop.lum
    │   └── functions.lum
    └── invalid/                  # Deliberately invalid programs (.lum)
```

---

## File Responsibilities

| File | Type | Primary Role | Key Functions / Structures |
| --- | --- | --- | --- |
| `src/main.c` | C Source | CLI Entry point, opens source file, drives the 4 compiler phases | `main()` |
| `src/lexer.l` | Flex File | Scans characters, matches regex, returns token IDs | `yylex()`, `yylineno` |
| `src/parser.y` | Bison File | Defines BNF grammar, builds AST, reports syntax errors | `yyparse()`, `yyerror()` |
| `src/ast.h` | C Header | Defines node kinds, data types, and AST node structure | `AstNode`, `NodeKind`, `DataType` |
| `src/ast.c` | C Source | Constructor functions to allocate nodes, free AST, and print AST | `ast_new_*()`, `ast_print()`, `ast_free()` |
| `src/symtab.h` | C Header | Defines Symbol, Scope, and SymbolTable structures | `Symbol`, `Scope`, `SymbolTable` |
| `src/symtab.c` | C Source | Implements scoped symbol insertion, lookup, and scope push/pop | `symtab_insert()`, `symtab_lookup()` |
| `src/semantic.h` | C Header | Interface for semantic analysis | `semantic_check()` |
| `src/semantic.c` | C Source | Walks AST to verify types, declarations, and function signatures | `semantic_check()`, `check_expression()` |
| `src/codegen.h` | C Header | Interface for Three-Address Code generation | `codegen_generate()` |
| `src/codegen.c` | C Source | Walks AST to emit intermediate instructions (TAC) | `codegen_generate()`, `gen_expr()`, `gen_stmt()` |
| `src/interp.h` | C Header | Interface for direct in-memory AST execution | `interp_execute()` |
| `src/interp.c` | C Source | Executes Lumis AST instructions in-memory (`-r` / `--run`) | `interp_execute()`, `eval_expr()`, `exec_stmt()` |
| `src/c_backend.h` | C Header | Interface for C code generation & GCC invocation | `c_backend_compile_binary()` |
| `src/c_backend.c` | C Source | Translates AST to C and compiles to native binary (`-o`) | `c_backend_generate()`, `c_backend_compile_binary()` |
| `Makefile` | Build Script | Runs `bison`, `flex`, and `gcc` to produce `lumis` binary | `make`, `make clean`, `make test` |

---

## Data Flow Between Files

```text
[ source.lum ]
      │
      ▼
  lexer.l ──(tokens)──► parser.y ──(calls ast.c)──► AstNode (root)
                                                        │
                                                        ▼
                                                  semantic.c ◄──► symtab.c
                                                        │
                                                        ▼
                                                    codegen.c
                                                        │
                                                        ▼
                                                  [ TAC Output ]
```
