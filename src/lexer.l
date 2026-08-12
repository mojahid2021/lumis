%{
/* ============================================================================
 *  lexer.l — Flex Lexical Analyzer for Lumis Compiler
 * ============================================================================
 *
 *  CSE 314 COMPILER DESIGN CONCEPTS (VIVA / DEFENSE PREPARATION):
 *
 *  1. WHAT IS A LEXICAL ANALYZER (LEXER / SCANNER)?
 *     The lexer is the first phase of the compiler. It reads the source code as
 *     a linear stream of characters and groups them into meaningful tokens
 *     (keywords, identifiers, numbers, operators, punctuation) using Regular
 *     Expressions (Regex).
 *
 *  2. FLEX AND DETERMINISTIC FINITE AUTOMATA (DFA):
 *     Flex converts the regular expressions defined below into an optimized
 *     Deterministic Finite Automaton (DFA) state machine. Scanning executes in
 *     O(L) linear time with respect to source code length L.
 *
 *  3. TOKEN VALUES & YYVAL:
 *     Tokens without associated values return their integer ID directly (e.g., INT, IF).
 *     Tokens with values (numbers, strings, identifiers) store their parsed values inside
 *     the `yylval` union (defined by Bison in parser.tab.h) before returning the token ID.
 * ============================================================================ */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int lexical_errors = 0;
%}

%option noyywrap
%option yylineno

/* Regular Expression Definitions */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT_NUM     {DIGIT}+
FLOAT_NUM   {DIGIT}+\.{DIGIT}+
CHAR_LIT    '([^'\\\n]|\\.)'
STRING_LIT  \"([^"\\\n]|\\.)*\"

%%

 /* ---------- White space and Comments ---------- */
[ \t\r]+                { /* Skip whitespace */ }
\n                      { /* Handled by yylineno */ }

"//".*                  { /* Single-line comment: skip */ }

"/*"                    {
                            /* Block comment: skip until closing tag */
                            int c;
                            while ((c = input()) != 0) {
                                if (c == '*') {
                                    if ((c = input()) == '/') break;
                                    else unput(c);
                                }
                            }
                            if (c == 0) {
                                fprintf(stderr, "[Lexical Error] Line %d: Unterminated block comment\n", yylineno);
                                lexical_errors++;
                            }
                        }

 /* ---------- Keywords ---------- */
"int"                   { return INT; }
"float"                 { return FLOAT; }
"char"                  { return CHAR; }
"bool"                  { return BOOL; }
"string"                { return STRING; }
"void"                  { return VOID; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"for"                   { return FOR; }
"return"                { return RETURN; }
"print"                 { return PRINT; }
"true"                  { yylval.intval = 1; return TRUE_LIT; }
"false"                 { yylval.intval = 0; return FALSE_LIT; }

 /* ---------- Operators and Punctuation ---------- */
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return STAR; }
"/"                     { return SLASH; }
"%"                     { return PERCENT; }

"=="                    { return EQ; }
"!="                    { return NEQ; }
"<="                    { return LE; }
">="                    { return GE; }
"<"                     { return LT; }
">"                     { return GT; }

"&&"                    { return AND; }
"||"                    { return OR; }
"!"                     { return NOT; }

"="                     { return ASSIGN; }
";"                     { return SEMI; }
","                     { return COMMA; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }

 /* ---------- Identifiers and Literals ---------- */
{INT_NUM}               {
                            yylval.intval = atoi(yytext);
                            return INT_NUM;
                        }

{FLOAT_NUM}             {
                            yylval.floatval = atof(yytext);
                            return FLOAT_NUM;
                        }

{CHAR_LIT}              {
                            yylval.charval = yytext[1];
                            return CHAR_LIT;
                        }

{STRING_LIT}            {
                            int len = strlen(yytext);
                            char *buf = (char *)malloc(len);
                            int j = 0;
                            for (int i = 1; i < len - 1; i++) {
                                if (yytext[i] == '\\' && i + 1 < len - 1) {
                                    i++;
                                    if (yytext[i] == 'n') buf[j++] = '\n';
                                    else if (yytext[i] == 't') buf[j++] = '\t';
                                    else if (yytext[i] == 'r') buf[j++] = '\r';
                                    else if (yytext[i] == '"') buf[j++] = '"';
                                    else if (yytext[i] == '\\') buf[j++] = '\\';
                                    else buf[j++] = yytext[i];
                                } else {
                                    buf[j++] = yytext[i];
                                }
                            }
                            buf[j] = '\0';
                            yylval.strval = buf;
                            return STRING_LIT;
                        }

{ID}                    {
                            yylval.strval = strdup(yytext);
                            return ID;
                        }

 /* ---------- Catch-all for unrecognized characters ---------- */
.                       {
                            fprintf(stderr, "[Lexical Error] Line %d: Unrecognized character '%s'\n", yylineno, yytext);
                            lexical_errors++;
                        }

%%
