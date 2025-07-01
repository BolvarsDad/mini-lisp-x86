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
    Purpose: Implements the AST (Abstract Sytax Tree) for x86_64 code generation.
*/

#include "ast.h"
#include "lexer.h"
#include "hashmap.h"

struct node *
node_create(struct token *t)
{
    struct node *n = malloc(sizeof(struct node));
    if (!n) return NULL;

    if (t.type == LPAREN) {
        n->type           = NODE_LIST;
        n->list->capacity = 16;
        n->list->count    = 0;
        n->list->children = (struct node **)calloc(n->list->capacity, sizeof(struct node *));

        return n;
    }

    n->type = NODE_ATOM;
    n->atom->token = t;

    return n;
}


