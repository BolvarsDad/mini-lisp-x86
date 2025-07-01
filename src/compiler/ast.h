#ifndef MINI_LISP_X86_SRC_COMPILER_AST_H_
#define MINI_LISP_X86_SRC_COMPILER_AST_H_

enum nodetype {
    NODE_PROGRAM,
    NODE_INVALID,
    NODE_LPAREN,
    NODE_RPAREN,
    NODE_SYMBOL,
    NODE_KEYWORD,
    NODE_STRING,
    NODE_NUMERIC,
    NODE_COMMENT,
};

struct node {
    enum nodetype type;

    union data {
        struct node_atom atom;
        struct node_list list;
    };
};

struct node_atom {
    struct token token;
};

struct node_list {
    struct node **children;

    size_t count;
    size_t capacity;
};

#endif
