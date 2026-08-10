/* =============================================================
 *  c_backend.c — C Backend implementation for Lumis
 * ============================================================= */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "c_backend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *c_type_name(DataType t) {
    switch (t) {
        case TYPE_INT:    return "int";
        case TYPE_FLOAT:  return "double";
        case TYPE_CHAR:   return "char";
        case TYPE_BOOL:   return "bool";
        case TYPE_STRING: return "const char*";
        case TYPE_VOID:   return "void";
        default:          return "int";
    }
}

static const char *c_binop_str(BinOp op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ:  return "==";
        case OP_NEQ: return "!=";
        case OP_LT:  return "<";
        case OP_GT:  return ">";
        case OP_LE:  return "<=";
        case OP_GE:  return ">=";
        case OP_AND: return "&&";
        case OP_OR:  return "||";
        default:     return "+";
    }
}

static void gen_c_expr(AstNode *node, FILE *out);
static void gen_c_stmt(AstNode *node, FILE *out, int indent);

static void print_indent(FILE *out, int indent) {
    for (int i = 0; i < indent; i++) fputs("    ", out);
}

static void gen_c_expr(AstNode *node, FILE *out) {
    if (!node) return;

    switch (node->kind) {
        case NODE_LITERAL:
            switch (node->data_type) {
                case TYPE_INT:    fprintf(out, "%d", node->int_value); break;
                case TYPE_FLOAT:  fprintf(out, "%g", node->float_value); break;
                case TYPE_CHAR:   fprintf(out, "'%c'", node->char_value); break;
                case TYPE_BOOL:   fprintf(out, "%s", node->bool_value ? "true" : "false"); break;
                case TYPE_STRING: fprintf(out, "\"%s\"", node->string_value ? node->string_value : ""); break;
                default:          fprintf(out, "0"); break;
            }
            break;

        case NODE_VAR_REF:
            fprintf(out, "%s", node->name);
            break;

        case NODE_UNARY_OP:
            if (node->unop == OP_NEG) fputs("-", out);
            else if (node->unop == OP_NOT) fputs("!", out);
            fputs("(", out);
            gen_c_expr(node->children[0], out);
            fputs(")", out);
            break;

        case NODE_BINARY_OP:
            if (node->data_type == TYPE_STRING && node->binop == OP_ADD) {
                fputs("__lumis_concat(__lumis_to_str(", out);
                gen_c_expr(node->children[0], out);
                fputs("), __lumis_to_str(", out);
                gen_c_expr(node->children[1], out);
                fputs("))", out);
            } else if ((node->children[0]->data_type == TYPE_STRING || node->children[1]->data_type == TYPE_STRING) &&
                       (node->binop == OP_EQ || node->binop == OP_NEQ || node->binop == OP_LT || node->binop == OP_GT || node->binop == OP_LE || node->binop == OP_GE)) {
                switch (node->binop) {
                    case OP_EQ:  fputs("__lumis_streq(", out); break;
                    case OP_NEQ: fputs("__lumis_strneq(", out); break;
                    case OP_LT:  fputs("__lumis_strlt(", out); break;
                    case OP_GT:  fputs("__lumis_strgt(", out); break;
                    case OP_LE:  fputs("__lumis_strle(", out); break;
                    case OP_GE:  fputs("__lumis_strge(", out); break;
                    default: break;
                }
                fputs("__lumis_to_str(", out);
                gen_c_expr(node->children[0], out);
                fputs("), __lumis_to_str(", out);
                gen_c_expr(node->children[1], out);
                fputs("))", out);
            } else {
                fputs("(", out);
                gen_c_expr(node->children[0], out);
                fprintf(out, " %s ", c_binop_str(node->binop));
                gen_c_expr(node->children[1], out);
                fputs(")", out);
            }
            break;

        case NODE_CALL:
            fprintf(out, "%s(", node->name);
            for (int i = 0; i < node->arg_count; i++) {
                if (i > 0) fputs(", ", out);
                gen_c_expr(node->args[i], out);
            }
            fputs(")", out);
            break;

        default:
            break;
    }
}

static void gen_c_stmt(AstNode *node, FILE *out, int indent) {
    if (!node) return;

    switch (node->kind) {
        case NODE_BLOCK:
            print_indent(out, indent);
            fputs("{\n", out);
            for (int i = 0; i < node->child_count; i++) {
                gen_c_stmt(node->children[i], out, indent + 1);
            }
            print_indent(out, indent);
            fputs("}\n", out);
            break;

        case NODE_VAR_DECL:
            print_indent(out, indent);
            if (node->child_count > 0) {
                fprintf(out, "%s %s = ", c_type_name(node->data_type), node->name);
                gen_c_expr(node->children[0], out);
                fputs(";\n", out);
            } else {
                if (node->data_type == TYPE_STRING) {
                    fprintf(out, "%s %s = \"\";\n", c_type_name(node->data_type), node->name);
                } else {
                    fprintf(out, "%s %s = 0;\n", c_type_name(node->data_type), node->name);
                }
            }
            break;

        case NODE_ASSIGN:
            print_indent(out, indent);
            fprintf(out, "%s = ", node->name);
            gen_c_expr(node->children[0], out);
            fputs(";\n", out);
            break;

        case NODE_IF:
            print_indent(out, indent);
            fputs("if (", out);
            gen_c_expr(node->children[0], out);
            fputs(")\n", out);
            gen_c_stmt(node->children[1], out, indent);
            if (node->child_count > 2) {
                print_indent(out, indent);
                fputs("else\n", out);
                gen_c_stmt(node->children[2], out, indent);
            }
            break;

        case NODE_WHILE:
            print_indent(out, indent);
            fputs("while (", out);
            gen_c_expr(node->children[0], out);
            fputs(")\n", out);
            gen_c_stmt(node->children[1], out, indent);
            break;

        case NODE_FOR:
            print_indent(out, indent);
            fputs("{\n", out);
            gen_c_stmt(node->children[0], out, indent + 1); /* init */
            print_indent(out, indent + 1);
            fputs("while (", out);
            gen_c_expr(node->children[1], out); /* cond */
            fputs(") {\n", out);
            gen_c_stmt(node->children[3], out, indent + 2); /* body */
            gen_c_stmt(node->children[2], out, indent + 2); /* update */
            print_indent(out, indent + 1);
            fputs("}\n", out);
            print_indent(out, indent);
            fputs("}\n", out);
            break;

        case NODE_RETURN:
            print_indent(out, indent);
            if (node->child_count > 0) {
                fputs("return ", out);
                gen_c_expr(node->children[0], out);
                fputs(";\n", out);
            } else {
                fputs("return;\n", out);
            }
            break;

        case NODE_PRINT:
            print_indent(out, indent);
            fputs("__lumis_print(", out);
            gen_c_expr(node->children[0], out);
            fputs(");\n", out);
            break;

        case NODE_CALL:
            print_indent(out, indent);
            gen_c_expr(node, out);
            fputs(";\n", out);
            break;

        case NODE_FUNC_DECL:
            fprintf(out, "%s %s(", c_type_name(node->return_type), node->name);
            for (int i = 0; i < node->param_count; i++) {
                if (i > 0) fputs(", ", out);
                fprintf(out, "%s %s", c_type_name(node->params[i]->data_type), node->params[i]->name);
            }
            fputs(") {\n", out);
            for (int i = 0; i < node->child_count; i++) {
                gen_c_stmt(node->children[i], out, 1);
            }
            fputs("}\n\n", out);
            break;

        default:
            break;
    }
}

void c_backend_generate(AstNode *root, FILE *out) {
    if (!root) return;

    fputs("#include <stdio.h>\n", out);
    fputs("#include <stdbool.h>\n", out);
    fputs("#include <stdlib.h>\n", out);
    fputs("#include <string.h>\n\n", out);

    fputs("static void __lumis_print_int(int x) { printf(\"%d\\n\", x); }\n", out);
    fputs("static void __lumis_print_float(double x) { printf(\"%g\\n\", x); }\n", out);
    fputs("static void __lumis_print_char(char x) { printf(\"%c\\n\", x); }\n", out);
    fputs("static void __lumis_print_bool(bool x) { printf(\"%s\\n\", x ? \"true\" : \"false\"); }\n", out);
    fputs("static void __lumis_print_string(const char *x) { printf(\"%s\\n\", x); }\n\n", out);

    fputs("static const char *__lumis_to_string_int(int x) { char *b = (char*)malloc(32); snprintf(b, 32, \"%d\", x); return b; }\n", out);
    fputs("static const char *__lumis_to_string_float(double x) { char *b = (char*)malloc(32); snprintf(b, 32, \"%g\", x); return b; }\n", out);
    fputs("static const char *__lumis_to_string_char(char x) { char *b = (char*)malloc(2); b[0]=x; b[1]='\\0'; return b; }\n", out);
    fputs("static const char *__lumis_to_string_bool(bool x) { return x ? \"true\" : \"false\"; }\n", out);
    fputs("static const char *__lumis_to_string_str(const char *x) { return x; }\n\n", out);

    fputs("#define __lumis_to_str(x) _Generic((x), \\\n", out);
    fputs("    bool: __lumis_to_string_bool, \\\n", out);
    fputs("    int: __lumis_to_string_int, \\\n", out);
    fputs("    double: __lumis_to_string_float, \\\n", out);
    fputs("    float: __lumis_to_string_float, \\\n", out);
    fputs("    char: __lumis_to_string_char, \\\n", out);
    fputs("    char*: __lumis_to_string_str, \\\n", out);
    fputs("    const char*: __lumis_to_string_str, \\\n", out);
    fputs("    default: __lumis_to_string_int)(x)\n\n", out);

    fputs("static const char *__lumis_concat(const char *s1, const char *s2) {\n", out);
    fputs("    char *res = (char*)malloc(strlen(s1) + strlen(s2) + 1);\n", out);
    fputs("    strcpy(res, s1); strcat(res, s2); return res;\n", out);
    fputs("}\n", out);
    fputs("static bool __lumis_streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }\n", out);
    fputs("static bool __lumis_strneq(const char *s1, const char *s2) { return strcmp(s1, s2) != 0; }\n", out);
    fputs("static bool __lumis_strlt(const char *s1, const char *s2) { return strcmp(s1, s2) < 0; }\n", out);
    fputs("static bool __lumis_strgt(const char *s1, const char *s2) { return strcmp(s1, s2) > 0; }\n", out);
    fputs("static bool __lumis_strle(const char *s1, const char *s2) { return strcmp(s1, s2) <= 0; }\n", out);
    fputs("static bool __lumis_strge(const char *s1, const char *s2) { return strcmp(s1, s2) >= 0; }\n\n", out);

    fputs("#define __lumis_print(x) _Generic((x), \\\n", out);
    fputs("    bool: __lumis_print_bool, \\\n", out);
    fputs("    int: __lumis_print_int, \\\n", out);
    fputs("    double: __lumis_print_float, \\\n", out);
    fputs("    float: __lumis_print_float, \\\n", out);
    fputs("    char: __lumis_print_char, \\\n", out);
    fputs("    char*: __lumis_print_string, \\\n", out);
    fputs("    const char*: __lumis_print_string, \\\n", out);
    fputs("    default: __lumis_print_int)(x)\n\n", out);

    /* Function forward declarations */
    for (int i = 0; i < root->child_count; i++) {
        AstNode *child = root->children[i];
        if (child->kind == NODE_FUNC_DECL) {
            fprintf(out, "%s %s(", c_type_name(child->return_type), child->name);
            for (int p = 0; p < child->param_count; p++) {
                if (p > 0) fputs(", ", out);
                fprintf(out, "%s %s", c_type_name(child->params[p]->data_type), child->params[p]->name);
            }
            fputs(");\n", out);
        }
    }
    fputs("\n", out);

    /* Function definitions */
    for (int i = 0; i < root->child_count; i++) {
        gen_c_stmt(root->children[i], out, 0);
    }
}

int c_backend_compile_binary(AstNode *root, const char *output_binary) {
    char tmp_c_path[256];
    snprintf(tmp_c_path, sizeof(tmp_c_path), "/tmp/lumis_tmp_%d.c", rand() % 1000000);

    FILE *f = fopen(tmp_c_path, "w");
    if (!f) {
        perror("Error creating temporary C source file");
        return 1;
    }

    c_backend_generate(root, f);
    fclose(f);

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gcc -O2 %s -o %s", tmp_c_path, output_binary);
    int res = system(cmd);

    remove(tmp_c_path);
    return res;
}
