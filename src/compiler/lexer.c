/*
    mini-lisp-x86 - A compiler for a subset of Common Lisp to x86_64
    Copyright (C) 2025 BolvarsDad

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. 
*/

#include "lexer.h"

#define MAX_TOKS 1024

struct token {
    char           *lexeme;
    int             line;
    int             column;
    enum toktype    type;
};

struct token tok_info_map[] = {
    {TOK_LPAREN, "TOK_LPAREN", "("},
    {TOK_RPAREN, "TOK_RPAREN", ")"},
    {TOK_DOT,    "TOK_DOT",    "."},
    {TOK_SYMBOL, "TOK_SYMBOL", NULL},
    {TOK_STRING, "TOK_STRING", NULL},
    {TOK_NUMBER, "TOK_NUMBER", NULL}
};

struct token *
lex(char const *src)
{
}
