# Mini Lisp Compiler (x86_64)  
A simple Common-Lisp-to-x86_64 compiler written in C, from scratch.  
Currently in early development — supports lexing parentheses, symbols, strings, and numeric literals.

---

## Features (Stage 1)
- Tokenizes basic Lisp syntax:
  - `(`, `)`
  - Symbols (e.g., `format`, `t`)
  - Strings (`"hello world"`)
  - Operators: `+`, `-`, `*`, `/`, `.`
- Line-based input reading

## TBI:
- Comments
- Functions
- Numbers (Integers, floats, and fractions)
---

### Requirements
- GCC or any C99-compatible compiler
- Standard C library (no external dependencies)

### Compile and Run

```bash
gcc main.c compiler/lexer.c -o minilisp
./minilisp tests/stage_1/hello.lisp
```
