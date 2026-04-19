/*
** client.c -- a stream socket client demo
*/
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "gkab_arena.h"
#include "net.h"
#include "byte_array.h"
#include "gkab_string.h"
#include "http.h"

struct gkab_arena g_arena;

int main(int argc, char *argv[])
{
	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

    gkab_arena_init(&g_arena);
    struct http http;
    http_init(&http);

    const char* out_filepath = "output";

    

    struct gkab_string url = gkab_string_dup_cstring(argv[1], &g_arena);
    struct gkab_string host;
    net_get_url_host(&url, &host, &g_arena);

    struct gkab_string port;
    if (!net_get_url_port(&url, &port, &g_arena)) {
        port = gkab_string_dup_cstring("80", &g_arena);
    }

    struct gkab_string scheme;
    if (!net_get_url_scheme(&url, &scheme, &g_arena)) {
        FILE *f = fopen(out_filepath, "w");
        fprintf(f, "INVALIDPROTOCOL");
        fclose(f);
        exit(0);
    }

    int sockfd;
    bool status = net_tcp_connection(&host, &port, &sockfd);

    if (!status) {
        FILE *f = fopen(out_filepath, "w");
        fprintf(f, "NOCONNECTION");
        fclose(f);
        return 0;
    }

    printf("sending http request...\n");

    struct gkab_string method = gkab_string_dup_cstring("GET", &g_arena);
    struct gkab_string path;
    if (!net_get_url_path(&url, &path, &g_arena)) {
        path = gkab_string_dup_cstring("/", &g_arena);
    }
    
    struct gkab_string protocol_version = gkab_string_dup_cstring("HTTP/1.1", &g_arena);


    struct gkab_string user_agent = gkab_string_dup_cstring("cs435-client", &g_arena);
    struct gkab_string accept = gkab_string_dup_cstring("*/*", &g_arena);
    
    //host = string_concat(&host, ":", &port, &g_arena);
    host = gkab_string_concat_cstring(&host, ":", &g_arena);
    host = gkab_string_concat(&host, &port, &g_arena);
    struct gkab_string connection = gkab_string_dup_cstring("Keep-Alive", &g_arena);

    struct http_message request;
    http_init_request(&request);
    http_set_start_line(&request, &method, &path, &protocol_version);
    http_set_header_field(&request, HF_USER_AGENT, &user_agent);
    http_set_header_field(&request, HF_ACCEPT, &accept);
    http_set_header_field(&request, HF_HOST, &host);
    http_set_header_field(&request, HF_CONNECTION, &connection);
    http_send_message(sockfd, &request, &g_arena);

    struct http_message response;
    http_init_response(&response);
    http_recv_message(sockfd, &response, &g_arena);

    FILE *f = fopen(out_filepath, "wb");
    if (http_response_status_code(&response) == SC_404) {
        fwrite("FILENOTFOUND", sizeof(u8), strlen("FILENOTFOUND"), f);
    } else if (http_response_status_code(&response) == SC_200) {
        fwrite(response.content.ptr, sizeof(u8), response.content.len, f);
    } else {
        //TODO should handle other statuses
        assert(false);
    }

    fclose(f);

    close(sockfd);

    gkab_arena_free(&g_arena);

    return 0;
}

