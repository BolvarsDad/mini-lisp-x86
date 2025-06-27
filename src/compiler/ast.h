#ifndef MINI_LISP_X86_SRC_COMPILER_AST_H_
#define MINI_LISP_X86_SRC_COMPILER_AST_H_

struct node {
    enum nodetype {
        NODE_EXPRESSION,
        NODE_IDENTIFIER,
        NODE_CONSTANT,
        NODE_FACTOR,
        NODE_TERM
    };

    size_t nchild;
    size_t maxchild;
};

#endif
