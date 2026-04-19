#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "gkab_arena.h"

typedef uint8_t u8;

struct byte_array {
    u8 *ptr;
    size_t capacity;
    off_t len;
};

void byte_array_init(struct byte_array *arr, size_t len, struct gkab_arena *arena) {
    arr->capacity = len;
    arr->ptr = gkab_arena_malloc(arena, len);
    arr->len = 0;
}

void byte_array_append_cstring(struct byte_array *arr, const char *s) {
    assert(arr->len + strlen(s) <= arr->capacity);
    size_t len = strlen(s);
    memcpy(arr->ptr + arr->len, s, len);
    arr->len += len;
}

void byte_array_append_bytes(struct byte_array *to, struct byte_array *from, size_t off, size_t count) {
    assert(off +count <= from->len);
    memcpy(to->ptr + to->len, from->ptr + off, count);
    to->len += count;
}

int byte_array_find(struct byte_array *arr, struct byte_array *bytes, int start) {
    u8 *p = arr->ptr + start;

    while (*p != '\0') {
        if (memcmp(p, bytes->ptr, bytes->len) == 0) {
            return p - arr->ptr;
        }
        p++;
    }
    
    return -1;
}

#endif //BYTE_ARRAY
