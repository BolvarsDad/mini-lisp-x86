/*
    mini-lisp-x86 - A compiler for a subset of Common Lisp to x86_64
    Copyright (C) 2025 Sinan Olsson-Pasic

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

    File: main.c
    Purpose: Entry point for the mlispc compiler driver.
*/

#define _POSIX_C_SOURCE 200809L // fork/exec/readlink for the -o link step

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <unistd.h>
#include <sys/wait.h>

#include "./compiler/lexer.h"
#include "./compiler/token_stream.h"
#include "./compiler/parser.h"
#include "./compiler/semantic.h"
#include "./compiler/ir.h"
#include "./compiler/codegen.h"

#include "./util/error.h"

#define VERSION "1.0"

static void
usage(FILE *stream)
{
    fprintf(stream, "MlispC - A minimal compiler from Common Lisp to x86_64\n");
    fprintf(stream, "usage: mlispc [options] <file.lisp>\n");
    fprintf(stream, "options:\n");
    fprintf(stream, "  -d, --debug          dump IR to stdout, annotate assembly with IR comments\n");
    fprintf(stream, "  -o, --output <file>  also assemble and link an executable to <file>\n");
    fprintf(stream, "  -v, --version        show version info\n");
    fprintf(stream, "  -h, --help           show this help\n");
}

char *
read_file(char const *filename)
{
    FILE *fp = fopen(filename, "r");
    long flen;
    char *buffer;

    if (fp == NULL) {
        fprintf(stderr, "minilisp: Cannot open file `%s`.\n", filename);
        perror("Error");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    flen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = malloc(flen + 1); // file contents + '\0'

    if (buffer == NULL) {
        fprintf(stderr, "minilisp: Cannot allocate memory.\n");
        perror("Error");
        return NULL;
    }

    fread(buffer, 1, flen, fp);
    buffer[flen] = '\0';
    fclose(fp);

    return buffer;
}

/* ! returns malloc'd buffer: "<input>.lisp" -> "<input>.s",
 * any other extension just gets ".s" appended */
static char *
derive_asm_path(const char *input)
{
    size_t len = strlen(input);
    const char *ext = ".lisp";
    size_t ext_len = strlen(ext);

    if (len >= ext_len && strcmp(input + len - ext_len, ext) == 0)
        len -= ext_len;

    char *path = malloc(len + 3); // ".s" + NUL
    if (path == NULL)
        return NULL;

    memcpy(path, input, len);
    memcpy(path + len, ".s", 3);

    return path;
}

static int
emit_asm_file(const struct ir_program *ir, const char *asm_path, int annotate)
{
    FILE *out = fopen(asm_path, "w");
    if (out == NULL) {
        fprintf(stderr, "minilisp: Cannot open output file `%s`.\n", asm_path);
        perror("Error");
        return 1;
    }

    int status = 0;

    if (codegen_emit(ir, out, annotate) != 0) {
        fprintf(stderr, "minilisp: Failed to write `%s`.\n", asm_path);
        status = 1;
    } else {
        printf("Assembly successfully written to %s\n", asm_path);
    }

    fclose(out);

    return status;
}

/* Locate build/runtime/runtime.o relative to the lispc binary itself
 * (via /proc/self/exe) so linking works from any working directory. */
static int
find_runtime_object(char *buf, size_t bufsize)
{
    ssize_t n = readlink("/proc/self/exe", buf, bufsize - 1);
    if (n < 0)
        return -1;
    buf[n] = '\0';

    char *slash = strrchr(buf, '/');
    if (slash == NULL)
        return -1;
    slash[1] = '\0';

    const char *rel = "build/runtime/runtime.o";
    if (strlen(buf) + strlen(rel) + 1 > bufsize)
        return -1;
    strcat(buf, rel);

    return 0;
}

/* Assemble and link the generated .s against the runtime. gcc is a
 * subprocess implementation detail here; this seam is where our own
 * assembler/linker slots in later. */
static int
link_executable(const char *asm_path, const char *out_path)
{
    char runtime_path[PATH_MAX];

    if (find_runtime_object(runtime_path, sizeof runtime_path) != 0) {
        fprintf(stderr, "minilisp: Cannot locate runtime object relative to lispc.\n");
        return 1;
    }

    if (access(runtime_path, R_OK) != 0) {
        fprintf(stderr, "minilisp: Runtime object `%s` not found; run `make` in src/.\n",
                runtime_path);
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("minilisp: fork");
        return 1;
    }

    if (pid == 0) {
        execlp("gcc", "gcc", asm_path, runtime_path, "-o", out_path, (char *)NULL);
        perror("minilisp: exec gcc");
        _exit(127);
    }

    int wstatus;

    if (waitpid(pid, &wstatus, 0) < 0) {
        perror("minilisp: waitpid");
        return 1;
    }

    if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != 0) {
        fprintf(stderr, "minilisp: Linking `%s` failed.\n", out_path);
        return 1;
    }

    printf("Executable written to %s\n", out_path);

    return 0;
}

int
main(int argc, char **argv)
{
    static const struct option long_options[] = {
        { "debug",   no_argument,       NULL, 'd' },
        { "output",  required_argument, NULL, 'o' },
        { "version", no_argument,       NULL, 'v' },
        { "help",    no_argument,       NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    int opt;
    int debug = 0;
    const char *out_path = NULL;

    while ((opt = getopt_long(argc, argv, "do:vh", long_options, NULL)) != -1) {
        switch (opt) {
        case 'd':
            debug = 1;
            break;
        case 'o':
            out_path = optarg;
            break;
        case 'v':
            printf("mlispc version %s\n", VERSION);
            return 0;
        case 'h':
            usage(stdout);
            return 0;
        default: /* '?': getopt has already printed a diagnostic */
            usage(stderr);
            return 1;
        }
    }

    if (optind != argc - 1) {
        usage(stderr);
        return 1;
    }

    const char *source_path = argv[optind];
    char *source = read_file(source_path);

    if (source == NULL)
        return 1;

    struct token_array tokens;
    struct token t;

    token_array_init(&tokens);

    struct lexer l = lexer_create(source, strlen(source));
    init_token_handlers();

    while ((t = lexer_next(&l)).type != TOK_END) {
        if (t.type != TOK_COMMENT)
            token_array_append(&tokens, t);
    }

    struct token_stream *ts = token_stream_create(&tokens);

    struct error_ctx *errctx = error_ctx_new(0); // default 32
    int status = 0;

    struct ast_node *program = parse_program(ts, errctx);

    if (errctx->count == 0)
        analyze_program(program, errctx);

    if (errctx->count == 0) {
        struct ir_program *ir = ir_program_new();

        if (ir == NULL) {
            fprintf(stderr, "Fatal: Unable to allocate IR program\n");
            status = 1;
        } else {
            if (translate_program(program, ir, errctx) != 0) {
                status = 1;
            } else {
                if (debug)
                    ir_program_print(ir, stdout);

                char *asm_path = derive_asm_path(source_path);

                if (asm_path == NULL) {
                    fprintf(stderr, "minilisp: Cannot allocate memory.\n");
                    status = 1;
                } else {
                    status = emit_asm_file(ir, asm_path, debug);

                    if (status == 0 && out_path != NULL)
                        status = link_executable(asm_path, out_path);

                    free(asm_path);
                }
            }

            ir_program_free(ir);
        }
    }

    if (errctx->count != 0) {
        error_ctx_print(errctx);
        status = 1;
    }

    ast_node_free(program);
    free(ts);
    free(source);
    token_array_free(&tokens);
    error_ctx_free(errctx);

    return status;
}
