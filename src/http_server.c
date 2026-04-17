/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <assert.h>

#include "http.h"

int default_handler(struct http_message *response, struct http_message *request, struct arena *arena) {
    if (http_write_file(response, request->start_line[RL_PATH_IDX].ptr + 1, arena)) {
        http_set_response_status(response, SC_200, arena);
    } else {
        http_set_response_status(response, SC_404, arena);
    }

    return 0;
}

int home(struct http_message *response, struct http_message *request, struct arena *arena) {
    if (http_write_file(response, "home.html", arena)) {
        http_set_response_status(response, SC_200, arena);
    } else {
        http_set_response_status(response, SC_404, arena);
    }

    return 0;
}

int about(struct http_message *response, struct http_message *request, struct arena *arena) {
    if (http_write_file(response, "about.html", arena)) {
        http_set_response_status(response, SC_200, arena);
    } else {
        http_set_response_status(response, SC_404, arena);
    }

    return 0;
}

int post(struct http_message *response, struct http_message *request, struct arena *arena) {
    if (http_write_file(response, "post.html", arena)) {
        http_set_response_status(response, SC_200, arena);
    } else {
        http_set_response_status(response, SC_404, arena);
    }

    return 0;
}

int not_found(struct http_message *response, struct http_message *request, struct arena *arena) {
    if (http_write_file(response, "not_found.html", arena)) {
        http_set_response_status(response, SC_404, arena);
    } else {
        http_set_response_status(response, SC_404, arena);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
	    fprintf(stderr,"usage: server port\n");
	    exit(1);
    }

    struct http http;
    http_init(&http);


    http_set_route(&http, "/", home);
    http_set_route(&http, "/about", about);
    http_set_route(&http, "/post", post);
    http_set_default_route(&http, default_handler);

    struct string port = string_from_cstring(argv[1], &http.arena);
    http_listen_and_serve(&http, &port);

	return 0;
}

