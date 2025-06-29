#ifndef MINI_LISP_X86_SRC_COMPILER_AST_H_
#define MINI_LISP_X86_SRC_COMPILER_AST_H_

enum nodetype {
    NODE_PROGRAM,
    NODE_LIST,
    NODE_SYMBOL,
    NODE_NUMBER,
    NODE_STRING,
    NODE_KEYWORD,
    NODE_DEFUN,
    NODE_DEFMACRO,
    NODE_LAMBDA,
    NODE_LET,
    NODE_IF,
    NODE_WHEN,
    NODE_DEFINE,
    NODE_CALL,
    NODE_QUOTE,
    NODE_ERROR 
}

struct node {
    enum nodetype type;
    struct token token;

    size_t nchildren;
    size_t capacity;
    char *content;

    struct node *children;
};

#endif
