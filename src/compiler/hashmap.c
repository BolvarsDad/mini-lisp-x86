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

#include "hashmap.h"

/*
 * U32HASHTABLE_LOADFACTOR - calculates the current load factor for a hash table
 * @n: number of entries currently occupied in the table, defined as a `size_t`
 * @m: total number of buckets, defined as a `size_t`
 */
#define U32HASHTABLE_LOADFACTOR(n, m) \
    ((n) / (m))

/*
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

/*
 * hash_fnv1a_32 - compute 32-bit FNV-1a hash of a NUL-terminated string
 * @str: input string to hash
 */
uint32_t
hash_fnv1a_32(const char *str)
{
    uint32_t hash = 2166136261u; /* FNV offset basis */

    while (*str)
        hash = (hash ^ (unsigned char)*str++) * 16777619; /* XOR then multiply by FNV prime */

    return hash;
}

/*
 *  u32hashmap_create - initializes the empty table and allocates space on the heap for buckets.
 *  start with something 
 *  @capacity: initial capacity for the table, 
 */
struct u32hashmap *
u32hashmap_create(size_t capacity)
{
    struct u32hashmap *map = malloc(sizeof(struct u32hashmap));
    if (!map) return NULL;

    map->count = 0;
    map->capacity = capacity;

    map->entries = (struct u32bucket **)calloc(map->capacity, sizeof(struct u32bucket *));
    if (!map->entries) return NULL;

    return map;
}

int
u32hashmap_insert(struct u32hashmap *map, char const *key, uint32_t val)
{
    if (!map || !key) return 0;

    uint32_t hash   = hash_fnv1a_32(key);
    size_t   idx    = hash % map->capacity;

    if ((double)U32HASHMAP_LOADFACTOR(map->count, map->capacity) >= 0.7)
        u32hashmap_resize(map);

    // linear probing
    for (size_t i = 0; i < map->capacity; ++i) {
        size_t probe = (idx + i) % map->capacity;
        struct u32bucket *bucket = map->entries[probe];

        if (bucket == NULL || bucket->status == BUCKET_EMPTY || bucket->status == BUCKET_DELETED) {
            struct u32bucket *new = malloc(sizeof(struct u32bucket));
            if (!new) return 0;

            new->key    = key; // does not copy the key
            new->val    = val;
            new->status = BUCKET_FILLED;

            map->entries[probe] = new;
            map->count++;

            return 1;
        }

        // overwrite the existing entry
        if (bucket->status == BUCKET_FILLED && strcmp(bucket->key, key) == 0) {
            bucket->val = val;

            return 1;
        }
    }

    return 0;
}

int
u32hashmap_delete(struct u32hashmap *map, char const *key)
{
    if (!map || !key) return 0;

    uint32_t hash = hash_fnv1a_32(key);
    size_t idx = hash % map->capacity;

    for (size_t i = 0; i < map->capacity; ++i) {
        size_t probe = (idx + i) % map->capacity;
        struct u32bucket *bucket = map->entries[probe];
        
        if (!bucket) return 0;

        if (bucket->status == BUCKET_FILLED && strcmp(bucket->key, key) == 0) {
            bucket->status = BUCKET_DELETED;
            map->count--;
            
            return 1;
        }
    }

    return 0;
}

int
u32hashmap_resize(struct u32hashmap *map)
{
    if (!map) return 0;

    // keep a history of the current buckets and reintroduce them to the new entries array
    // with u32hashmap_insert()
    struct u32bucket **entries_old = map->entries;
    struct u32bucket **entries_new = (struct u32bucket **)calloc(map->capacity * 2, sizeof(struct u32bucket *));
    if (!entries_new) return 0;

    map->entries    = entries_new;
    map->capacity  *= 2;
    map->count      = 0;

    for (size_t i = 0; i < map->capacity; ++i)
    {
        struct u32bucket *bucket = entries_old[i];
        if (bucket != NULL && bucket->status == BUCKET_FILLED) {
            u32hashmap_insert(map, bucket->key, bucket->val);
            free(bucket);
        }
    }

    free(entries_old);
    
    return 1;
}
