#ifndef MINI_LISP_X86_SRC_COMPILER_BUILTIN_HASH_H_
#define MINI_LISP_X86_SRC_COMPILER_BUILTIN_HASH_H_

#include <stddef.h>
#include <string.h>

enum builtin_type {
    BUILTIN_NOT,
    BUILTIN_PLUS,
    BUILTIN_MINUS,
    BUILTIN_MUL,
    BUILTIN_DIV,
    BUILTIN_LT,
    BUILTIN_GT,
    BUILTIN_NUM_EQ,  // `=` is numeric equality; eql/equal are separate
    BUILTIN_IF,
    BUILTIN_LET,
    BUILTIN_QUOTE,
    BUILTIN_LAMBDA,
    BUILTIN_DEFUN,
    BUILTIN_FORMAT
};

const char *lookup_builtin(const char *str, size_t len);

// Convert matched string to our enum
static inline enum builtin_type builtin_kind(const char *str, size_t len)
{
    // Single-character operators are decided here, ahead of the perfect
    // hash: they are the most common form heads, and the switch is
    // cheaper than hashing plus a strcmp. Any other one-char symbol is a
    // variable, not a builtin. builtins.gperf still lists these so the
    // two stay in agreement.
    if (len == 1) {
        switch (str[0]) {
            case '+': return BUILTIN_PLUS;
            case '-': return BUILTIN_MINUS;
            case '*': return BUILTIN_MUL;
            case '/': return BUILTIN_DIV;
            case '<': return BUILTIN_LT;
            case '>': return BUILTIN_GT;
            case '=': return BUILTIN_NUM_EQ;
            default:  return BUILTIN_NOT;
        }
    }

    const char *matched = lookup_builtin(str, len);
    if (matched == NULL) {
        return BUILTIN_NOT;
    }

    // Longer names
    if (strcmp(matched, "if") == 0)     return BUILTIN_IF;
    if (strcmp(matched, "let") == 0)    return BUILTIN_LET;
    if (strcmp(matched, "quote") == 0)  return BUILTIN_QUOTE;
    if (strcmp(matched, "lambda") == 0) return BUILTIN_LAMBDA;
    if (strcmp(matched, "defun") == 0)  return BUILTIN_DEFUN;
    if (strcmp(matched, "format") == 0) return BUILTIN_FORMAT;

    return BUILTIN_NOT;  // Should never happen
}

#endif
