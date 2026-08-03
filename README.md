# Mini Lisp Compiler (x86_64)

A Common-Lisp-to-x86_64 compiler written in C from scratch — no external
dependencies, no compiler frameworks.

The full pipeline (lexer, parser, semantic analysis, three-address-code IR,
x86_64 codegen) works end-to-end for a growing language subset: source files
compile to native Linux executables.

---

## Quick start

```bash
make
cd src
```

```lisp
; average.lisp
(let ((a 10)
      (b 20))
  (/ (+ a b) 2))
```

```bash
$ ./lispc -o average average.lisp   # also writes average.s next to the source
$ ./average
15
```

`-d` additionally dumps the IR and annotates the generated assembly with
the IR it lowers.

A program returns the value of its last top-level form; the runtime prints
it. For the bundled test programs there's an equivalent make shortcut:

```bash
$ make tests/stage_2/01_variable_binding.bin
$ ./tests/stage_2/01_variable_binding.bin
30
```

x86_64 Linux only. Building the compiler requires GCC (or any C99
compiler); compiling lisp programs additionally needs binutils (`as` and
`ld`). The produced binaries are small static executables with no libc
dependency. `make clean` removes all build output, including generated
`.s`/`.bin` files.

---

## Language support

Working today:

- Integers, string literals, `nil`, `t`, `()`
- Arithmetic `+ - * /` with Common Lisp semantics (variadic left-fold,
  unary negation/reciprocal)
- `let` bindings, including shadowing and nested scopes
- `if`, one- and two-armed, arbitrarily nested (only `nil` is false)
- Comparison predicates `< > =`, n-ary as in Common Lisp (`(< 1 2 3)`)
- `format` to `t` with the `~a ~d ~% ~~` directives

Not yet: `cond`, `defun`/`lambda`, quote, lists, `setq`.
Unsupported forms are rejected with source-located diagnostics rather than
miscompiled.

---

## Project structure

```
src/
├── compiler/
│   ├── lexer.c       # Tokenization
│   ├── parser.c      # Parsing & AST construction
│   ├── ast.c         # AST node creation and management
│   ├── semantic.c    # Scope/arity validation, diagnostics
│   ├── ir.c          # AST → three-address code
│   └── codegen.c     # IR → x86_64 assembly
├── runtime/
│   └── runtime.c     # Process entry; prints the program's result
├── util/             # error context, strdup/strndup
├── tests/            # stage_{1,2,3}/*.lisp test programs
└── main.c            # driver
```

Design decisions are documented where they live: `compiler/ir.h` covers the
IR and the tagged value encoding, `compiler/codegen.h` the register
allocation strategy.

---

## Roadmap

- [x] Lexer with dispatch table
- [x] Token stream abstraction
- [x] AST generation
- [x] Semantic analysis
- [x] Intermediate representation (three-address code)
- [x] x86_64 code generation (spill everything)
- [x] Basic runtime system (result printing)
- [x] Driver produces executables directly (`-o`)
- [x] Freestanding runtime — no libc; programs assemble and link with
      plain `as` + `ld`
- [x] `format` and the builtin calling convention
- [x] Comparison predicates `< > =`
- [ ] Constant folding (AST → AST pass)
- [ ] Functions: `defun`, `lambda`, recursion
- [ ] Linear-scan register allocation
- [ ] Own assembler/linker — no GCC anywhere in the pipeline

---
