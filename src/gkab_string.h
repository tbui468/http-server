#ifndef GKAB_STRING
#define GKAB_STRING

#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "gkab_arena.h"

struct gkab_string {
    union {
        char *cstring;
        char *ptr;
    };
    size_t len;
};

#define gkab_string_empty() (struct gkab_string) { .ptr=NULL, .len=0 }

struct gkab_string gkab_string_dup_cstring(const char *ptr, struct gkab_arena *arena) {
    struct gkab_string s;
    s.len = strlen(ptr);
    s.cstring = gkab_arena_malloc(arena, s.len + 1);
    memcpy(s.cstring, ptr, s.len);
    s.cstring[s.len] = '\0';
    return s;
}


struct gkab_string gkab_string_dup(struct gkab_string *s, struct gkab_arena *arena) {
    return gkab_string_dup_cstring((const char *) s->cstring, arena);
}


struct gkab_string gkab_string_concat_cstring(struct gkab_string *first, const char *second, struct gkab_arena *arena) {
    struct gkab_string s;
    size_t len = strlen(second);
    s.ptr = gkab_arena_malloc(arena, first->len + len + 1);
    s.len = first->len + len;
    memcpy(s.ptr, first->ptr, first->len);
    memcpy(s.ptr + first->len, second, len);
    s.ptr[s.len] = '\0';
    return s;
}

struct gkab_string gkab_string_concat(struct gkab_string *first, struct gkab_string *second, struct gkab_arena *arena) {
    return gkab_string_concat_cstring(first, (const char *) second->cstring, arena);
}

int gkab_string_cmp_cstring(struct gkab_string *s1, const char *s2) {
    off_t off = 0;
    size_t s2_len = strlen(s2);
    while (off < s1->len && off < s2_len) {
        int c1 = (int) s1->ptr[off];
        int c2 = (int) s2[off];
        if (c1 != c2) {
            return c1 - c2;
        }
        off++;
    }

    if (s1->len == s2_len) {
        return 0;
    }

    if (s1->len < s2_len) {
        return -1;
    }

    return 1;
}

int gkab_string_cmp(struct gkab_string *s1, struct gkab_string *s2) {
    return gkab_string_cmp_cstring(s1, (const char *) s2->cstring);
}

int gkab_string_find_cstring(struct gkab_string *s, const char *substring, int start) {
    assert(start <= s->len);
    const char *p = s->ptr + start;
    size_t substring_len = strlen(substring);
    
    while (*p != '\0') {
        if (strncmp(p, substring, substring_len) == 0) {
            return p - s->ptr;
        }
        p++;
    }
    
    return -1;
}

int gkab_string_find(struct gkab_string *s1, struct gkab_string *s2, int start) {
    return gkab_string_find_cstring(s1, (const char *) s2->cstring, start);
}

struct gkab_string gkab_string_slice_charstar(char* s, int start, int end, struct gkab_arena *arena) {
    assert(start >= 0);
    assert(start <= end);

    struct gkab_string result;
    result.len = end - start;
    result.cstring = gkab_arena_malloc(arena, result.len + 1);
    memcpy(result.cstring, s + start, result.len);
    result.cstring[result.len] = '\0';
    
    return result;
}

struct gkab_string gkab_string_slice(struct gkab_string *s, int start, int end, struct gkab_arena *arena) {
    return gkab_string_slice_charstar(s->cstring, start, end, arena);
}


#endif //GKAB_STRING
