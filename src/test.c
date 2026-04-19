#include <stdio.h>
#include <assert.h>

#include "gkab_arena.h"
#include "gkab_string.h"
#include "net.h"

void string_find_cstring_test() {
    struct gkab_arena arena;
    gkab_arena_init(&arena);

    struct gkab_string s;

    s = gkab_string_dup_cstring("I like cats", &arena);
    assert(gkab_string_find_cstring(&s, "I", 0) == 0);

    s = gkab_string_dup_cstring("//http", &arena);
    assert(gkab_string_find_cstring(&s, "/http", 0) == 1);

    s = gkab_string_dup_cstring("asfdasdf", &arena);
    assert(gkab_string_find_cstring(&s, "/http", 0) == -1);

    s = gkab_string_dup_cstring("http://www.example.com/", &arena);
    assert(gkab_string_find_cstring(&s, "www.example.com", 0) == 7);

    s = gkab_string_dup_cstring("http://www.example.com", &arena);
    assert(gkab_string_find_cstring(&s, "www.example.com", 0) == 7);

    s = gkab_string_dup_cstring("http://www.example.com", &arena);
    assert(gkab_string_find_cstring(&s, ":", 0) == 4);

    s = gkab_string_dup_cstring("///htppppppppp", &arena);
    assert(gkab_string_find_cstring(&s, "/http", 0) == -1);

    s = gkab_string_dup_cstring("aabbbaa", &arena);
    assert(gkab_string_find_cstring(&s, "aa", 2) == 5);

    gkab_arena_free(&arena);
    printf("string_find_cstring_tests passed\n");
}

void net_get_url_scheme_test() {
    struct gkab_arena arena;
    gkab_arena_init(&arena);
    struct gkab_string result;

    struct gkab_string url = gkab_string_dup_cstring("http://www.example.com", &arena);
    assert(net_get_url_scheme(&url, &result, &arena));
    assert(result.len == 4);

    url = gkab_string_dup_cstring("http://example/sada/adfafd/adf", &arena);
    assert(net_get_url_scheme(&url, &result, &arena));
    assert(result.len == 4);

    url = gkab_string_dup_cstring("https://www.example.com", &arena);
    assert(!net_get_url_scheme(&url, &result, &arena));

    gkab_arena_free(&arena);
    printf("net_get_url_scheme_tests passed\n");
}


void net_get_url_host_test() {
    struct gkab_arena arena;
    gkab_arena_init(&arena);

    struct gkab_string result;
    struct gkab_string url;
    const char *host = "www.example.com";

    url = gkab_string_dup_cstring("http://www.example.com", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = gkab_string_dup_cstring("http://www.example.com/", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = gkab_string_dup_cstring("http://www.example.com/path/to/file", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = gkab_string_dup_cstring("http://www.example.com:443", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = gkab_string_dup_cstring("http://www.example.com:80/", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = gkab_string_dup_cstring("http://www.example.com:123/path/to/file", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    gkab_arena_free(&arena);
    printf("net_get_url_host_tests passed\n");
}

void net_get_url_port_test() {
    struct gkab_arena arena;
    gkab_arena_init(&arena);
    struct gkab_string result;
    struct gkab_string url;

    url = gkab_string_dup_cstring("http://www.example.com", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = gkab_string_dup_cstring("http://www.example.com/", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = gkab_string_dup_cstring("http://www.example.com/asdf/adfasdf", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = gkab_string_dup_cstring("http://www.example.com:443", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "443", result.len) == 0 && result.len == 3);

    url = gkab_string_dup_cstring("http://www.example.com:80/", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "80", result.len) == 0 && result.len == 2);

    url = gkab_string_dup_cstring("http://www.example.com:12345/path/to/file", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "12345", result.len) == 0 && result.len == 5);

    gkab_arena_free(&arena);
    printf("net_get_url_port_tests passed\n");
}


void net_get_url_path_test() {
    struct gkab_arena arena;
    gkab_arena_init(&arena);
    struct gkab_string result;
    struct gkab_string url;

    url = gkab_string_dup_cstring("http://www.example.com", &arena);
    assert(!net_get_url_path(&url, &result, &arena));

    url = gkab_string_dup_cstring("http://www.example.com/", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/", result.ptr, result.len) == 0 && result.len == 1);

    url = gkab_string_dup_cstring("http://www.example.com/aaaa/bbbb", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/aaaa/bbbb", result.ptr, result.len) == 0 && result.len == 10);

    url = gkab_string_dup_cstring("http://www.example.com:443", &arena);
    assert(!net_get_url_path(&url, &result, &arena));

    url = gkab_string_dup_cstring("http://www.example.com:80/", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/", result.ptr, result.len) == 0 && result.len == 1);

    url = gkab_string_dup_cstring("http://www.example.com:12345/a/b/c", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/a/b/c", result.ptr, result.len) == 0 && result.len == 6);

    gkab_arena_free(&arena);
    printf("net_get_url_path_tests passed\n");
}

int main() {
    string_find_cstring_test();
    net_get_url_scheme_test();
    net_get_url_host_test();
    net_get_url_port_test();
    net_get_url_path_test();
    return 0;
}
