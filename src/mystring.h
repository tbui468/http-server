#ifndef MYSTRING_H
#define MYSTRING_H

#include <string.h>
#include <stdbool.h>

#include "arena.h"

struct string {
    char *ptr; //TODO should call this cstring to be 100% clear it's null terminated
    size_t len;
};

#define string_empty() (struct string) { .ptr=NULL, .len=0 }

struct string string_init(const char *ptr, size_t len, struct arena *arena) {
    struct string s;
    s.ptr = arena_malloc(arena, len + 1);
    s.len = len;
    memcpy(s.ptr, ptr, len);
    s.ptr[len] = '\0';
    return s;
}

struct string string_from_cstring(const char *ptr, struct arena *arena) {
    return string_init(ptr, strlen(ptr), arena);
}

struct string string_from_int(int i, struct arena *arena) {
    char buf[16]; //should be big enough for any int
    sprintf(buf, "%d", i);
    return string_from_cstring(buf, arena);
}

struct string string_concat(struct string *first, const char *sep, struct string *second, struct arena *arena) {
    struct string s;
    int sep_len = strlen(sep);
    s.ptr = arena_malloc(arena, first->len + sep_len + second->len + 1);
    s.len = first->len + sep_len + second->len;
    memcpy(s.ptr, first->ptr, first->len);
    memcpy(s.ptr + first->len, sep, sep_len);
    memcpy(s.ptr + first->len + sep_len, second->ptr, second->len);
    s.ptr[s.len] = '\0';
    return s;
}

int string_find_cstring(struct string *s, const char *substring, int start) {
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

bool string_equals(struct string *s1, struct string *s2) {
    return s1->len == s2->len && memcmp(s1->ptr, s2->ptr, s1->len) == 0;
}

#endif //MYSTRING_H
