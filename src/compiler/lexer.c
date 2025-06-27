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

#define MAX_TOKS 256
#define LUTSZ 256

static struct token (*token_handlers[256])(struct lexer *) = {0};

void
init_token_handlers()
{
    token_handlers['('] = lex_paren;
    token_handlers[')'] = lex_paren;

    token_handlers['"'] = lex_string;

    token_handlers['+'] = lex_operator;
    token_handlers['-'] = lex_operator;
    token_handlers['*'] = lex_operator;
    token_handlers['/'] = lex_operator;

    token_handlers['#'] = lex_macro;
    token_handlers[':'] = lex_keyword;
    token_handlers[';'] = lex_comment;

    for (int i = '0'; i <= '9'; ++i)
        token_handlers[i] = lex_numeric;

    for (int i = 0; i < LUTSZ; ++i)
        if (token_handlers[i] == NULL)
            token_handlers[i] = lex_symbol;
}

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
            return ")";
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
        case TOK_MACRO:
            return "Macro";
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
lex_paren(struct lexer *l)
{
    struct token t;

    t.lexeme = &l->content[l->cursor];
    t.len    = 1;
    t.type   = (l->content[l->cursor] == '(') ? TOK_LPAREN : TOK_RPAREN;

    l->cursor++;

    return t;
}

struct token
lex_string(struct lexer *l)
{
    struct token t;
    size_t start = ++l->cursor;

    while (l->cursor < l->len && l->content[l->cursor] != '"')
        l->cursor++;

    t.lexeme    = &l->content[start];
    t.len       = l->cursor - start;
    t.type      = TOK_STRING;

    l->cursor++;

    return t;
}

struct token
lex_operator(struct lexer *l)
{
    struct token t;
    
    t.lexeme    = &l->content[l->cursor];
    t.len       = 1;
    t.type      = TOK_OPERATOR;

    l->cursor++;

    return t;
}

struct token
lex_numeric(struct lexer *l)
{
    struct token t;
    size_t start = l->cursor;

    while (isdigit(l->content[l->cursor]) || strchr("./", l->content[l->cursor]))
        l->cursor++;

    t.lexeme    = &l->content[start];
    t.len       = l->cursor - start;
    t.type      = TOK_NUMERIC;

    return t;
}

struct token
lex_macro(struct lexer *l)
{
    struct token t;
    size_t start = l->cursor;

    while (l->content[l->cursor] != '\n')
        l->cursor++;

    t.lexeme    = &l->content[start];
    t.len       = l->cursor - start;
    t.type      = TOK_MACRO;

    return t;
}

struct token
lex_keyword(struct lexer *l)
{
    struct token t;
    size_t start = l->cursor;

    while(l->cursor < l->len && l->content[l->cursor] != ' ')
        l->cursor++;

    t.lexeme    = &l->content[start];
    t.len       = l->cursor - start;
    t.type      = TOK_KEYWORD;

    return t;
}

struct token
lex_comment(struct lexer *l)
{
    struct token t;
    size_t start = l->cursor;

    // does not yet support multi-line comments
    while (l->cursor <= l->len && l->content[l->cursor] != '\n')
        l->cursor++;

    t.lexeme = &l->content[start];
    t.len = l->cursor - start;
    t.type = TOK_COMMENT;

    return t;
}

struct token
lex_symbol(struct lexer *l)
{
    struct token t;
    size_t start = l->cursor;

    while (l->cursor < l->len && is_symbol_char(l->content[l->cursor]))
        l->cursor++;

    t.lexeme = &l->content[start];
    t.len = l->cursor - start;
    t.type = TOK_SYMBOL;

    return t;
}

int
is_symbol_char(char c)
{
    return isalnum(c) || strchr("-_+*/:<=>!?&~^", c);
}

char
lexer_peek(struct lexer *l)
{
    if (l->cursor == l->len)
        return '\0';

    return l->content[l->cursor + 1];
}

struct token
lexer_next(struct lexer *l)
{
    struct token t;

    t.lexeme = NULL;
    t.len = 0;
    t.type = TOK_INVALID;

    while (l->cursor < l->len && isspace(l->content[l->cursor]))
        l->cursor++;

    if (l->cursor >= l->len) {
        t.lexeme = NULL;
        t.len    = 0;
        t.type   = TOK_END;

        return t;
    }

    char c = l->content[l->cursor];

    return token_handlers[(unsigned char)c](l);
}

void
tokenize(char const *line, size_t len)
{
    struct lexer l;
    struct token t;

    l = lexer_create(line, len);
    init_token_handlers();

    while ((t = lexer_next(&l)).type != TOK_END) {
        printf("Token: %-10s | ", token_to_str(t.type));
        printf("Lexeme: %.*s\n", (int)t.len, t.lexeme);
    }
}
