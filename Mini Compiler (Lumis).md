# Mini Compiler (Lumis)


## **1. Introduction** {#introduction}

A compiler is a program that translates source code written in a high-level language into a lower-level form, such as assembly or intermediate code, while checking that the program is syntactically and semantically valid along the way. Building a compiler, even a small one, requires bringing together the core theoretical ideas of the Compiler Design course --- regular expressions and finite automata, context-free grammars, parsing strategies, type checking, and code generation --- into a single working system.

This project proposes the design and implementation of a Mini Compiler that accepts a simplified programming language and processes it through the classical compiler pipeline: lexical analysis, syntax analysis, semantic analysis, and code generation. The project is implemented in C using Flex (Lex) for tokenization and Bison (Yacc) for parsing, giving the team direct, hands-on experience with the same tool families used in many real-world compiler front ends.

## **2. Problem Statement** {#problem-statement}

Understanding compiler theory from lectures and textbooks alone leaves a gap between abstract concepts (grammars, parse trees, symbol tables) and the concrete engineering decisions needed to make them work together. There is a need for a small, self-contained compiler that:

- Demonstrates each phase of compilation as a distinct, inspectable stage.

- Accepts a well-defined subset of a C-like language, small enough to implement in a semester project.

- Detects and reports lexical, syntax, and semantic errors clearly.

- Produces working intermediate or assembly code that can be traced back to the original source.

## **3. Objectives** {#objectives}

The primary objectives of the Mini Compiler project are to:

- Design a token specification and implement a lexical analyzer using Flex.

- Define a context-free grammar for the supported language and implement a parser using Bison.

- Build an Abstract Syntax Tree (AST) representation of parsed programs.

- Implement semantic analysis, including symbol table management, type checking, and scope resolution.

- Generate intermediate code (e.g., three-address code) and/or target assembly code from the validated AST.

- Provide clear, descriptive error messages at each phase (lexical, syntax, and semantic errors).

- Test the compiler against a suite of sample programs covering both valid and invalid inputs.

## **4. Scope of the Supported Language** {#scope-of-the-supported-language}

The Mini Compiler will support a simplified, C-like language rather than the full C standard, keeping the grammar tractable within the project timeline. Planned language features include:

- Primitive data types: int, float, char, and bool.

- Variable declarations and assignment statements.

- Arithmetic, relational, and logical expressions with standard operator precedence.

- Control structures: if / else, while, and for loops.

- Function definitions and calls, with parameter passing and return values.

- Single-line and block comments, ignored by the lexer.

Features such as pointers, structures, arrays with multiple dimensions, and the preprocessor are treated as stretch goals and may be added if time permits, but are not required for the core deliverable.

## **5. Tools and Technologies** {#tools-and-technologies}

The project will be implemented entirely in C, using the standard compiler-construction toolchain taught in the course:

- Flex (Fast Lexical Analyzer Generator) --- used to specify token patterns with regular expressions and generate the lexical analyzer (scanner).

- Bison (GNU Yacc) --- used to specify the grammar and generate the LALR parser, including semantic action hooks for building the AST.

- C (GCC) --- the implementation language for the AST, symbol table, semantic analysis routines, and code generator.

- Make --- to automate the build process across the lexer, parser, and supporting C source files.

- Git --- for version control and collaborative development.

## **6. System Architecture - The Compiler Pipeline** {#system-architecture---the-compiler-pipeline}

The Mini Compiler follows the traditional four-phase compiler pipeline, with each phase consuming the output of the previous one:

### **6.1 Lexical Analysis (Flex)** {#lexical-analysis-flex}

The lexer scans the raw source file character by character and groups characters into tokens (keywords, identifiers, literals, operators, punctuation) based on regular expressions defined in a .l file. It strips whitespace and comments, tracks line numbers for error reporting, and passes a stream of tokens to the parser.

### **6.2 Syntax Analysis (Bison)** {#syntax-analysis-bison}

The parser consumes the token stream and matches it against a context-free grammar defined in a .y file. As grammar rules are reduced, semantic actions build an Abstract Syntax Tree (AST) representing the structure of the program. Syntax errors (e.g., missing semicolons, unmatched braces) are detected and reported with line information at this stage.

### **6.3 Semantic Analysis** {#semantic-analysis}

The AST is traversed to enforce rules that context-free grammars cannot express: type checking, variable declaration-before-use, scope resolution, and function signature matching. A symbol table is built and consulted during this traversal, and semantic errors (e.g., type mismatches, undeclared variables, redeclaration) are reported.

### **6.4 Code Generation, Execution & Binary Output** {#code-generation}

Once a program passes semantic analysis, the compiler supports three output modes:
1. **Three-Address Code (TAC):** Walks the AST to emit intermediate instructions.
2. **In-Memory AST Execution (`-r` / `--run`):** Interprets and executes Lumis program instructions directly.
3. **Native Binary Compilation (`-o <output>`):** Translates AST into C and invokes GCC to produce native executable binaries.

## **7. Methodology and Work Plan** {#methodology-and-work-plan}

The project will be developed incrementally, phase by phase, so that each stage can be tested independently before integration:

| **Wk** | **Task** | **Phase** | **Deliverable** |
|----|----|----|----|
| 1 | Define language grammar; specify tokens and regular expressions | Planning | Grammar & token spec |
| 2 | Implement lexer in Flex; unit test tokenization | Lexical Analysis | Working scanner |
| 3 | Implement parser in Bison; build AST; test with sample programs | Syntax Analysis | Parser + AST |
| 3.5 | Build symbol table; implement type checking and scope rules | Semantic Analysis | Semantic checker |
| 4 | Implement intermediate/assembly code generation from AST | Code Generation | Code generator |
| 4.2 | Integration testing, error-handling polish, sample test suite | Testing | Test report |
| 4.5 | Documentation and final report / presentation | Wrap-up | Final submission |

## **8. Expected Outcomes** {#expected-outcomes}

- A functioning Mini Compiler that reads a source file and reports lexical, syntax, and semantic errors with line numbers.

- Correct intermediate or assembly code generated for valid programs.

- A test suite of sample programs demonstrating each phase, including deliberately invalid inputs to show error detection.

- A clear mapping between compiler-theory concepts covered in lectures and their implementation in Flex/Bison/C.

- Documentation describing the grammar, token set, symbol table design, and code generation strategy.

## **9. Conclusion** {#conclusion}

This Mini Compiler project translates the theoretical foundations of the Compiler Design course into a concrete, working system. By implementing the full pipeline lexer, parser, semantic analyzer, and code generator using Flex, Bison, and C, the project reinforces core concepts such as finite automata, context-free grammars, symbol tables, and code generation, while producing a practical artifact that can be extended in future coursework or personal projects.
