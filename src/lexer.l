%{
/* =============================================================
 *  lexer.l — Flex Lexical Analyzer for Lumis
 * =============================================================
 *  Scans the input stream character by character and turns it
 *  into a stream of tokens for Bison (parser.y).
 *
 *  Tracks line numbers automatically via %option yylineno.
 *  Saves literal values into yylval union members defined in
 *  parser.y (parser.tab.h).
 * ============================================================= */

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
