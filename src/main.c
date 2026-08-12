/* ============================================================================
 *  main.c — Main CLI Driver & Pipeline Orchestrator for Lumis Compiler
 * ============================================================================
 *
 *  Interpreter Mode (`-r` or default):
 *  Parses input source code, performs semantic checking, and executes
 *  the AST in-memory using the interpreter engine.
 * ============================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"
#include "semantic.h"
#include "interp.h"

extern FILE *yyin;
extern int yyparse(void);
extern int yylex(void);
extern char *yytext;
extern int yylineno;
extern AstNode *ast_root;
extern int lexical_errors;
extern int syntax_errors;

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [-r|--run] <source-file.lum>\n", prog_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -r, --run              Compile and execute the program in memory\n");
    fprintf(stderr, "  -h, --help             Show this help message\n");
}

int main(int argc, char **argv) {
    const char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
            /* Option -r is accepted for explicit execution mode */
            continue;
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

    /* 1. Lexical & Syntax Analysis */
    if (yyparse() != 0 || lexical_errors > 0 || syntax_errors > 0 || !ast_root) {
        fprintf(stderr, "\n❌ Compilation failed during Lexical/Syntax Analysis (%d lexical, %d syntax errors).\n",
                lexical_errors, syntax_errors);
        fclose(file);
        return 1;
    }

    /* 2. Semantic Analysis & Symbol Table */
    SymbolTable symtab;
    symtab_init(&symtab);
    int sem_errors = semantic_check(ast_root, &symtab);

    if (sem_errors > 0) {
        fprintf(stderr, "❌ Compilation failed: %d semantic error(s) detected.\n", sem_errors);
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return 1;
    }

    /* 3. Program Execution via Interpreter (-r) */
    printf("=== PROGRAM EXECUTION ===\n");
    int exit_code = interp_execute(ast_root);
    printf("=========================\n");
    printf("Program finished with exit code %d\n", exit_code);

    symtab_free(&symtab);
    ast_free(ast_root);
    fclose(file);
    return exit_code;
}
