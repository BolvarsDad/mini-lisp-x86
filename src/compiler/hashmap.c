/*
    mini-lisp-x86 - A compiler for a subset of Common Lisp to x86_64
    Copyright (C) 2025 BolvarsDad

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

    File: hashmap.c
    Purpose: Implements a hashmap..
*/

#include <stdio.h>
#include <stdint.h>

// #define TABLESZ 128

/*
 *
 * UNUSED: Might be enabled later on for benchmarking purposes.
 *
unsigned int
hash_djb2(const char *str)
{
    unsigned int hash = 5381;

    while (*str)
        hash = ((hash << 5) + hash) + (unsigned char)*str++;

    return hash % TABLESZ;
}

unsigned long
hash_sdbm(const char *str)
{
    unsigned long hash = 0;

    while (*str)
        hash = *str++ + (hash << 6) + (hash << 16) - hash;

    return hash;
}
*/

uint32_t
hash_fnv1a(const char *str)
{
    uint32_t hash = 2166136261u;

    while (*str)
        hash = (hash ^ (unsigned char)*str++) * 16777619;

    return hash;
}

int main(int argc, char **argv)
{
    const char *str = "+";
    uint32_t res = hash_fnv1a(str);

    printf("Str: %s\nHash: %u\n", str, res);

    return 0;
}
