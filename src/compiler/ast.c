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

    File: ast.c
    Purpose: Implements the AST (Abstract Sytax Tree) for x86_64 code generation using BNF.
    BNF: https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form
*/

#include "ast.h"
#include "lexer.h"

struct node *
node_create(struct token *t, enum nodetype type)
{
    struct node *n = calloc(1, sizeof(struct node));
    if (n == NULL)
        return NULL;

    n->type = type;
    n->token = *t;

    n->nchildren = 0;
    n->capacity = 0;
    n->content = t->lexeme;

    n->children = NULL;

    return n;
}

void
print_ast(struct *node root)
{
    switch (root->type) {
        case NODE_LIST:
            printf("LIST:\n");

            for (size_t i = 0; i < root->nchildren; ++i)
                print_ast(n->children[i]);

            break;

        case NODE_SYMBOL:
            printf("SYMBOL: %.*s\n", (int)root->token.len, root->token.lexeme);
            break;

        case NODE_NUMERIC:
            printf("NUMERIC: %.*s\n", (int)root->token.len, root->token.lexeme);
            break;
    }
}
