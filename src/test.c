#include <stdio.h>
#include <assert.h>

#include "arena.h"
#include "mystring.h"
#include "net.h"

void string_find_cstring_test() {
    struct arena arena;
    arena_init(&arena);

    struct string s;

    s = string_from_cstring("I like cats", &arena);
    assert(string_find_cstring(&s, "I", 0) == 0);

    s = string_from_cstring("//http", &arena);
    assert(string_find_cstring(&s, "/http", 0) == 1);

    s = string_from_cstring("asfdasdf", &arena);
    assert(string_find_cstring(&s, "/http", 0) == -1);

    s = string_from_cstring("http://www.example.com/", &arena);
    assert(string_find_cstring(&s, "www.example.com", 0) == 7);

    s = string_from_cstring("http://www.example.com", &arena);
    assert(string_find_cstring(&s, "www.example.com", 0) == 7);

    s = string_from_cstring("http://www.example.com", &arena);
    assert(string_find_cstring(&s, ":", 0) == 4);

    s = string_from_cstring("///htppppppppp", &arena);
    assert(string_find_cstring(&s, "/http", 0) == -1);

    s = string_from_cstring("aabbbaa", &arena);
    assert(string_find_cstring(&s, "aa", 2) == 5);

    arena_free(&arena);
    printf("string_find_cstring_tests passed\n");
}

void net_get_url_scheme_test() {
    struct arena arena;
    arena_init(&arena);
    struct string result;

    struct string url = string_from_cstring("http://www.example.com", &arena);
    assert(net_get_url_scheme(&url, &result, &arena));
    assert(result.len == 4);

    url = string_from_cstring("http://example/sada/adfafd/adf", &arena);
    assert(net_get_url_scheme(&url, &result, &arena));
    assert(result.len == 4);

    url = string_from_cstring("https://www.example.com", &arena);
    assert(!net_get_url_scheme(&url, &result, &arena));

    arena_free(&arena);
    printf("net_get_url_scheme_tests passed\n");
}


void net_get_url_host_test() {
    struct arena arena;
    arena_init(&arena);

    struct string result;
    struct string url;
    const char *host = "www.example.com";

    url = string_from_cstring("http://www.example.com", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = string_from_cstring("http://www.example.com/", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = string_from_cstring("http://www.example.com/path/to/file", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = string_from_cstring("http://www.example.com:443", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = string_from_cstring("http://www.example.com:80/", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    url = string_from_cstring("http://www.example.com:123/path/to/file", &arena);    
    assert(net_get_url_host(&url, &result, &arena));
    assert(strncmp(result.ptr, host, result.len) == 0 && result.len == strlen(host));

    arena_free(&arena);
    printf("net_get_url_host_tests passed\n");
}

void net_get_url_port_test() {
    struct arena arena;
    arena_init(&arena);
    struct string result;
    struct string url;

    url = string_from_cstring("http://www.example.com", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = string_from_cstring("http://www.example.com/", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = string_from_cstring("http://www.example.com/asdf/adfasdf", &arena);
    assert(!net_get_url_port(&url, &result, &arena));

    url = string_from_cstring("http://www.example.com:443", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "443", result.len) == 0 && result.len == 3);

    url = string_from_cstring("http://www.example.com:80/", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "80", result.len) == 0 && result.len == 2);

    url = string_from_cstring("http://www.example.com:12345/path/to/file", &arena);
    assert(net_get_url_port(&url, &result, &arena));
    assert(strncmp(result.ptr, "12345", result.len) == 0 && result.len == 5);

    arena_free(&arena);
    printf("net_get_url_port_tests passed\n");
}


void net_get_url_path_test() {
    struct arena arena;
    arena_init(&arena);
    struct string result;
    struct string url;

    url = string_from_cstring("http://www.example.com", &arena);
    assert(!net_get_url_path(&url, &result, &arena));

    url = string_from_cstring("http://www.example.com/", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/", result.ptr, result.len) == 0 && result.len == 1);

    url = string_from_cstring("http://www.example.com/aaaa/bbbb", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/aaaa/bbbb", result.ptr, result.len) == 0 && result.len == 10);

    url = string_from_cstring("http://www.example.com:443", &arena);
    assert(!net_get_url_path(&url, &result, &arena));

    url = string_from_cstring("http://www.example.com:80/", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/", result.ptr, result.len) == 0 && result.len == 1);

    url = string_from_cstring("http://www.example.com:12345/a/b/c", &arena);
    assert(net_get_url_path(&url, &result, &arena));
    assert(strncmp("/a/b/c", result.ptr, result.len) == 0 && result.len == 6);

    arena_free(&arena);
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
