/* =============================================================
 *  main.c — Driver for Lumis Mini Compiler
 * =============================================================
 *  Coordinates the classical 4-phase compilation pipeline:
 *     1. Lexical & Syntax Analysis (Flex & Bison -> AST)
 *     2. AST Pretty-Printing
 *     3. Semantic Analysis & Symbol Table Generation
 *     4. Three-Address Code (TAC) Generation / Execution / C Binary
 * ============================================================= */

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
extern AstNode *ast_root;
extern int lexical_errors;
extern int syntax_errors;

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [options] <source-file.lum>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -r, --run              Compile and execute the program immediately\n");
    fprintf(stderr, "  -o <output_binary>     Compile program to a native executable binary using GCC\n");
    fprintf(stderr, "  -h, --help             Show this help message\n");
}

int main(int argc, char **argv) {
    int run_mode = 0;
    const char *out_binary = NULL;
    const char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
            run_mode = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
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

    yyin = file;

    if (!run_mode && !out_binary) {
        printf("=========================================\n");
        printf("        LUMIS COMPILER PIPELINE\n");
        printf("=========================================\n\n");
        printf("Compiling source file: %s\n\n", filename);
    }

    /* Phase 1 & 2: Lexical and Syntax Analysis */
    if (!run_mode && !out_binary) {
        printf("[1/4] Parsing source code and building AST...\n");
    }
    if (yyparse() != 0 || lexical_errors > 0 || syntax_errors > 0 || !ast_root) {
        fprintf(stderr, "\n❌ Compilation failed during Lexical/Syntax Analysis (%d lexical, %d syntax errors).\n",
                lexical_errors, syntax_errors);
        fclose(file);
        return 1;
    }
    if (!run_mode && !out_binary) {
        printf("✓ Syntax analysis succeeded.\n\n");

        /* Print AST */
        printf("=== ABSTRACT SYNTAX TREE (AST) ===\n");
        ast_print(ast_root, stdout, 0);
        printf("\n");
    }

    /* Phase 3: Semantic Analysis & Symbol Table */
    if (!run_mode && !out_binary) {
        printf("[3/4] Running Semantic Analysis...\n");
    }
    SymbolTable symtab;
    symtab_init(&symtab);

    int sem_errors = semantic_check(ast_root, &symtab);

    if (!run_mode && !out_binary) {
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
    if (!run_mode && !out_binary) {
        printf("✓ Semantic analysis succeeded.\n\n");
    }

    /* Compile to native binary if -o flag specified */
    if (out_binary) {
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

    /* Execute program if -r flag specified */
    if (run_mode) {
        printf("=== PROGRAM EXECUTION ===\n");
        int exit_code = interp_execute(ast_root);
        printf("=========================\n");
        printf("Program finished with exit code %d\n", exit_code);

        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return exit_code;
    }

    /* Phase 4: Code Generation (Default Pipeline View) */
    printf("[4/4] Generating Three-Address Code (TAC)...\n");
    printf("\n=== THREE-ADDRESS CODE (TAC) ===\n");
    codegen_generate(ast_root, stdout);
    printf("\n");

    printf("=========================================\n");
    printf("✓ Compilation finished successfully!\n");
    printf("=========================================\n");

    /* Cleanup */
    symtab_free(&symtab);
    ast_free(ast_root);
    fclose(file);
    return 0;
}
