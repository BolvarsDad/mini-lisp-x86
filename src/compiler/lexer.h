#ifndef MINI_LISP_X86_COMPILER_LEXER_H_
#define MINI_LISP_X86_COMPILER_LEXER_H_

struct lexer    lexer_create(char const*src, size_t srclen);
char const     *lexer_trim_left(char const *str);
struct token    lexer_next(struct lexer *l);
void            tokenize(char const *line, size_t len);

enum toktype {
    TOK_END = 0,
    TOK_INVALID,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_SYMBOL,     // e.g. variable names
    TOK_KEYWORD,    // e.g. 'format'
    TOK_STRING,
    TOK_IDENTIFIER,
    TOK_INTEGER,
    TOK_FLOAT,
    TOK_FRACTION,
    TOK_OPERATOR
};

struct token {
    char const     *lexeme;
    size_t          len;
    enum toktype    type;
};

struct lexer {
    char const     *content;
    size_t          len;
    size_t          cursor;
};

#endif /* MINI_LISP_X86_COMPILER_LEXER_H_ */
