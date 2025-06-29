#ifndef MINI_LISP_X86_SRC_COMPILER_HASHMAP_H_
#define MINI_LISP_X86_SRC_COMPILER_HASHMAP_H_

struct u32bucket {
    char const  *val;
    uint32_t     key;
};

struct u32hashmap {
    size_t nbuckets;
    size_t capacity;

    struct u32bucket **entries;
};

#endif
