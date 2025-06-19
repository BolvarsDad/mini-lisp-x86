#ifndef MINI_LISP_X86_COMPILER_LEXER_H_
#define MINI_LISP_X86_COMPILER_LEXER_H_

enum toktype {
    TOK_LPAREN,     // '('
    TOK_RPAREN,     // ')'
    TOK_DOT,        // '.'
    TOK_SYMBOL,     // e.g. t, format
    TOK_STRING,     // "Hello World"
    TOK_NUMBER      // -2, -1, 0, 1, 2, 3, ...
};

#endif /* MINI_LISP_X86_COMPILER_LEXER_H_ */
