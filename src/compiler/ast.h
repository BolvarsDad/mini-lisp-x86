#ifndef MINI_LISP_X86_SRC_COMPILER_AST_C_
#define MINI_LISP_X86_SRC_COMPILER_AST_C_

struct node {
    enum nodetype {
        NODE_LIST,
        NODE_SYMBOL,
        NODE_STRING,
        NODE_INTEGER
    };

    size_t nchild;
    size_t maxchild;
};

#endif
