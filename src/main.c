/* =============================================================
 *  main.c — Driver for Lumis Mini Compiler
 * =============================================================
 *  Coordinates the classical 4-phase compilation pipeline:
 *     1. Lexical & Syntax Analysis (Flex & Bison -> AST)
 *     2. AST Pretty-Printing
 *     3. Semantic Analysis & Symbol Table Generation
 *     4. Three-Address Code (TAC) Generation
 * ============================================================= */

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symtab.h"
#include "semantic.h"
#include "codegen.h"

extern FILE *yyin;
extern int yyparse(void);
extern AstNode *ast_root;
extern int lexical_errors;
extern int syntax_errors;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file.lum>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    yyin = file;

    printf("=========================================\n");
    printf("        LUMIS COMPILER PIPELINE\n");
    printf("=========================================\n\n");
    printf("Compiling source file: %s\n\n", filename);

    /* Phase 1 & 2: Lexical and Syntax Analysis */
    printf("[1/4] Parsing source code and building AST...\n");
    if (yyparse() != 0 || lexical_errors > 0 || syntax_errors > 0 || !ast_root) {
        fprintf(stderr, "\n❌ Compilation failed during Lexical/Syntax Analysis (%d lexical, %d syntax errors).\n",
                lexical_errors, syntax_errors);
        fclose(file);
        return 1;
    }
    printf("✓ Syntax analysis succeeded.\n\n");

    /* Print AST */
    printf("=== ABSTRACT SYNTAX TREE (AST) ===\n");
    ast_print(ast_root, stdout, 0);
    printf("\n");

    /* Phase 3: Semantic Analysis & Symbol Table */
    printf("[3/4] Running Semantic Analysis...\n");
    SymbolTable symtab;
    symtab_init(&symtab);

    int sem_errors = semantic_check(ast_root, &symtab);

    printf("\n=== SYMBOL TABLE ===\n");
    symtab_print(&symtab, stdout);
    printf("\n");

    if (sem_errors > 0) {
        fprintf(stderr, "❌ Compilation failed: %d semantic error(s) detected.\n", sem_errors);
        symtab_free(&symtab);
        ast_free(ast_root);
        fclose(file);
        return 1;
    }
    printf("✓ Semantic analysis succeeded.\n\n");

    /* Phase 4: Code Generation */
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
