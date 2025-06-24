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

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"

#define MAX_TOKS 1024

char const *
token_to_str(enum toktype type)
{
    switch(type) {
        case TOK_END:
            return "End of content";
        case TOK_INVALID:
            return "Invalid token";
        case TOK_LPAREN:
            return "(";
        case TOK_RPAREN:
            return "')'";
        case TOK_SYMBOL:
            return "Symbol";
        case TOK_KEYWORD:
            return "Keyword";
        case TOK_STRING:
            return "String";
        case TOK_NUMERIC:
            return "Numeric";
        case TOK_OPERATOR:
            return "Operator";
        default:
            return NULL;
    }
    return NULL;
}

struct lexer
lexer_create(char const *src, size_t srclen)
{
    struct lexer l;

    l.content   = src;
    l.len       = srclen;
    l.cursor    = 0;

    return l;
}

struct token
lexer_next(struct lexer *l)
{
    struct token t;

    t.lexeme = NULL;
    t.len = 0;
    t.type = TOK_INVALID;

    if (l->cursor >= l->len) {
        t.type = TOK_END;
        return t; 
    }

    while (isspace(l->content[l->cursor]))
        l->cursor++;

    if (l->content[l->cursor]== '\0') {
        t.type = TOK_END;
        return t;
    }

    if (l->content[l->cursor]== '(') {
        t.type = TOK_LPAREN;
        t.lexeme = &l->content[l->cursor];
        t.len = 1;

        l->cursor++;

        return t;
    }

    if (l->content[l->cursor]== ')') {
        t.type = TOK_RPAREN;
        t.lexeme = &l->content[l->cursor];
        t.len = 1;

        l->cursor++;

        return t;
    }

    if (l->content[l->cursor]== '"') {
        l->cursor++;
        size_t start = l->cursor;

        while (l->cursor < l->len && l->content[l->cursor]!= '"' && l->content[l->cursor]!= '\0')
            l->cursor++;

        if (l->content[l->cursor]== '"') {
            t.type = TOK_STRING;
            t.lexeme = &l->content[start];
            t.len = l->cursor - start;

            l->cursor++;
        }

        else
            t.type = TOK_INVALID;

        return t;
    }

    if (strchr("+./*", l->content[l->cursor]) && l->content[l->cursor + 1] == ' ') {
        t.type = TOK_OPERATOR;
        t.lexeme = &l->content[l->cursor];
        t.len = 1;

        l->cursor++;

        return t;
    }

    if (isalpha(l->content[l->cursor])) {
        size_t start = l->cursor;

        while (isalnum(l->content[l->cursor])) {
            l->cursor++;
        }

        t.type = TOK_IDENTIFIER;
        t.lexeme = &l->content[start];
        t.len = l->cursor - start;

        return t;
    }

    
    if (isdigit(l->content[l->cursor])) {
        size_t start = l->cursor;

        while (isdigit(l->content[l->cursor])) {
            l->cursor++;

            if (strchr("./", l->content[l->cursor])) {
                l->cursor++;
            }
        }

        t.type = TOK_NUMERIC;
        t.lexeme = &l->content[start];
        t.len = l->cursor - start;

        return t;
    }
    

    t.type = TOK_INVALID;
    t.lexeme = &l->content[l->cursor];
    t.len = 1;

    l->cursor++;

    return t;
}

void
tokenize(char const *line, size_t len)
{
    struct lexer l;
    struct token t;

    l = lexer_create(line, len);

    while ((t = lexer_next(&l)).type != TOK_END) {
        printf("Token: %-10s | ", token_to_str(t.type));
        printf("Lexeme: %.*s\n", (int)t.len, t.lexeme);
    }
}
