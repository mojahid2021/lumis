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
│       └── 06_HOW_TO_RUN_AND_EXTEND.md
├── src/                          # Compiler Source Code (C, Flex, Bison)
│   ├── main.c                    # Driver entry point
│   ├── lexer.l                   # Flex scanner specification
│   ├── parser.y                  # Bison parser specification
│   ├── ast.h / ast.c             # Abstract Syntax Tree representation
│   ├── symtab.h / symtab.c       # Symbol Table & scope management
│   ├── semantic.h / semantic.c   # Type checking & semantic analyzer
│   └── codegen.h / codegen.c     # TAC code generator
└── tests/                        # Test suite
    ├── valid/                    # Valid programs (.lum)
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
