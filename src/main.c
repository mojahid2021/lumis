/* ============================================================================
 *  main.c — Main CLI Driver & Pipeline Orchestrator for Lumis Compiler
 * ============================================================================
 *
 *  CSE 314 COMPILER DESIGN CONCEPTS (VIVA / DEFENSE PREPARATION):
 *
 *  1. COMPILER PASSES & ARCHITECTURAL STAGES:
 *     - Lexical Diagnostic Mode (`-t`): Scans raw character stream into tokens.
 *     - Syntactic Structural Mode (`-p`): Parses tokens and builds AST hierarchy.
 *     - Semantic Environmental Mode (`-s`): Checks static types and prints Symbol Table.
 *     - Three-Address Code Mode (`-c`): Generates intermediate machine-independent TAC.
 *     - Interpreter Mode (`-r`): In-memory AST evaluation engine.
 *     - Native Compilation Mode (`-o <bin>`): Transpiles AST to C and builds GCC binary.
 *     - Default Pipeline (no flags): Runs full diagnostic compiler suite.
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "semantic.h"
#include "codegen.h"
#include "interp.h"
#include "c_backend.h"

extern FILE *yyin;
extern int yyparse(void);
extern int yylex(void);
extern char *yytext;
extern int yylineno;
extern AstNode *ast_root;
extern int lexical_errors;
extern int syntax_errors;

/* Diagnostic mode flags */
typedef enum {
    MODE_DEFAULT = 0,
    MODE_TOKENS  = 1,
    MODE_AST     = 2,
    MODE_SYMTAB  = 3,
    MODE_TAC     = 4,
    MODE_RUN     = 5,
    MODE_OUTPUT  = 6
} ExecutionMode;

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [options] <source-file.lum>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -t, --tokens           Dump scanned lexical tokens with line numbers\n");
    fprintf(stderr, "  -p, --ast              Pretty-print the parsed Abstract Syntax Tree (AST)\n");
    fprintf(stderr, "  -s, --symtab           Perform semantic check and display Symbol Table state\n");
    fprintf(stderr, "  -c, --tac              Generate Three-Address Code (TAC) intermediate code\n");
    fprintf(stderr, "  -r, --run              Compile and execute the program immediately in memory\n");
    fprintf(stderr, "  -o <output_binary>     Compile program to a native executable binary using GCC\n");
    fprintf(stderr, "  -h, --help             Show this help message\n");
}

/* Helper to dump scanned lexical tokens for -t mode */
static int run_lexer_diagnostic(FILE *file) {
    printf("=== LEXICAL TOKENS DIAGNOSTIC MODE (-t) ===\n");
    yyin = file;
    int tok;
    while ((tok = yylex()) != 0) {
        printf("Line %3d | Token ID: %-4d | Text: '%s'\n", yylineno, tok, yytext);
    }
    if (lexical_errors > 0) {
        fprintf(stderr, "❌ Lexical scanning completed with %d error(s).\n", lexical_errors);
        return 1;
    }
    printf("✓ Lexical scanning completed successfully.\n");
    return 0;
}

int main(int argc, char **argv) {
    ExecutionMode mode = MODE_DEFAULT;
    const char *out_binary = NULL;
    const char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
            mode = MODE_TOKENS;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--ast") == 0) {
            mode = MODE_AST;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symtab") == 0) {
            mode = MODE_SYMTAB;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--tac") == 0) {
            mode = MODE_TAC;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
            mode = MODE_RUN;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                mode = MODE_OUTPUT;
                out_binary = argv[++i];
            } else {
                fprintf(stderr, "Error: -o option requires an output binary name.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        print_usage(argv[0]);
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    /* 1. Lexer Diagnostic Mode (-t) */
    if (mode == MODE_TOKENS) {
        int res = run_lexer_diagnostic(file);
        fclose(file);
        return res;
    }

    yyin = file;

    if (mode == MODE_DEFAULT) {
        printf("=========================================\n");
        printf("        LUMIS COMPILER PIPELINE\n");
        printf("=========================================\n\n");
        printf("Compiling source file: %s\n\n", filename);
        printf("[1/4] Parsing source code and building AST...\n");
    }

    /* 2. Lexical & Syntax Analysis */
    if (yyparse() != 0 || lexical_errors > 0 || syntax_errors > 0 || !ast_root) {
        fprintf(stderr, "\n❌ Compilation failed during Lexical/Syntax Analysis (%d lexical, %d syntax errors).\n",
                lexical_errors, syntax_errors);
        fclose(file);
        return 1;
    }

    if (mode == MODE_AST) {
        printf("=== ABSTRACT SYNTAX TREE (AST) ===\n");
        ast_print(ast_root, stdout, 0);
        ast_free(ast_root);
        fclose(file);
        return 0;
    }

    if (mode == MODE_DEFAULT) {
        printf("✓ Syntax analysis succeeded.\n\n");
        printf("=== ABSTRACT SYNTAX TREE (AST) ===\n");
        ast_print(ast_root, stdout, 0);
        printf("\n");
        printf("[3/4] Running Semantic Analysis...\n");
    }

    /* 3. Semantic Analysis & Symbol Table */
    SymbolTable symtab;
    symtab_init(&symtab);
    int sem_errors = semantic_check(ast_root, &symtab);

    if (mode == MODE_SYMTAB) {
        printf("=== SYMBOL TABLE ===\n");
        symtab_print(&symtab, stdout);
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return (sem_errors > 0) ? 1 : 0;
    }

    if (mode == MODE_DEFAULT) {
        printf("\n=== SYMBOL TABLE ===\n");
        symtab_print(&symtab, stdout);
        printf("\n");
    }

    if (sem_errors > 0) {
        fprintf(stderr, "❌ Compilation failed: %d semantic error(s) detected.\n", sem_errors);
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return 1;
    }

    if (mode == MODE_DEFAULT) {
        printf("✓ Semantic analysis succeeded.\n\n");
    }

    /* 4. Execution / Native Build / TAC Emission */
    if (mode == MODE_OUTPUT) {
        int build_res = c_backend_compile_binary(ast_root, out_binary);
        if (build_res == 0) {
            printf("✓ Built executable binary '%s' successfully.\n", out_binary);
        } else {
            fprintf(stderr, "❌ Failed to build binary '%s'.\n", out_binary);
        }
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return build_res;
    }

    if (mode == MODE_RUN) {
        printf("=== PROGRAM EXECUTION ===\n");
        int exit_code = interp_execute(ast_root);
        printf("=========================\n");
        printf("Program finished with exit code %d\n", exit_code);

        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return exit_code;
    }

    if (mode == MODE_TAC) {
        printf("=== THREE-ADDRESS CODE (TAC) ===\n");
        codegen_generate(ast_root, stdout);
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return 0;
    }

    /* Default pipeline */
    printf("[4/4] Generating Three-Address Code (TAC)...\n");
    printf("\n=== THREE-ADDRESS CODE (TAC) ===\n");
    codegen_generate(ast_root, stdout);
    printf("\n");
    printf("=========================================\n");
    printf("✓ Compilation finished successfully!\n");
    printf("=========================================\n");

    symtab_free(&symtab);
    ast_free(ast_root);
    fclose(file);
    return 0;
}
