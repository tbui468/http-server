#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include "byte_array.h"
#include "gkab_string.h"
#include "net.h"

#define MAX_HTTP_HEADER_SIZE 8192
#define CRLF_SIZE 2
#define SPACE_SIZE 1
#define COLON_SIZE 1

#define RL_METHOD_IDX 0
#define RL_PATH_IDX 1
#define RL_HTTP_VERSION_IDX 2

#define SL_HTTP_VERSION_IDX 0
#define SL_STATUS_CODE_IDX 1
#define SL_REASON_PHRASE_IDX 2

#define START_LINE_FIELD_COUNT 3

struct byte_array g_space_bytes;
struct byte_array g_single_crlf_bytes;
struct byte_array g_double_crlf_bytes;
bool g_initialized = false;

struct http_message;

struct http_route {
    struct gkab_string name;
    int (*handler)(struct http_message *response, struct http_message *request, struct gkab_arena*);
};

#define MAX_ROUTES 16

struct gkab_string string_from_int(int i, struct gkab_arena *arena) {
    char buf[16];
    sprintf(buf, "%d", i);
    return gkab_string_dup_cstring(buf, arena);
}

struct http {
    struct gkab_arena arena;
    struct http_route routes[MAX_ROUTES];
    struct http_route default_route;
    int route_count;
};

//Ensure this matches enum http_header_field_names below
enum http_header_field {
    HF_USER_AGENT = 0,
    HF_ACCEPT,
    HF_HOST,
    HF_CONNECTION,
    HF_ACCEPT_ENCODING,
    HF_CONTENT_LENGTH,

    HF_COUNT //sentinel value
};

//Ensure this matches enum http_header_field above
struct gkab_string http_header_field_names[HF_COUNT] = {
    { .ptr="User-Agent", .len=10 },
    { .ptr="Accept", .len=6 },
    { .ptr="Host", .len=4 },
    { .ptr="Connection", .len=10 },
    { .ptr="Accept-Encoding", .len=15 },
    { .ptr="Content-Length", .len=14 }
};

enum http_status_code {
    SC_200 = 200,
    SC_404 = 404
};

const char* http_status_code_to_reason_phrase(enum http_status_code code) {
    switch (code) {
    case SC_200:
        return "OK";
    case SC_404:
        return "Not found";
    default:
        assert(false);
        return "error";
    }
}


size_t http_start_line_get_size(struct gkab_string start_line[START_LINE_FIELD_COUNT]) {
    return start_line[0].len + 
           SPACE_SIZE + 
           start_line[1].len + 
           SPACE_SIZE + 
           start_line[2].len + 
           CRLF_SIZE;
}

size_t http_header_fields_get_size(struct gkab_string header_fields[HF_COUNT]) {
    size_t size = 0;
    for (int i = 0; i < HF_COUNT; i++) {
        if (header_fields[i].len != 0) {
            size += http_header_field_names[i].len + COLON_SIZE + SPACE_SIZE + header_fields[i].len + CRLF_SIZE;
        }
    }
    return size;
}

void write_header_field(struct byte_array *arr, struct gkab_string *key, struct gkab_string *value) {
    byte_array_append_cstring(arr, key->ptr);
    byte_array_append_cstring(arr, ": ");
    byte_array_append_cstring(arr, value->ptr);
    byte_array_append_cstring(arr, "\r\n");
}

enum http_message_type {
    MT_REQUEST = 0,
    MT_RESPONSE
};

struct http_message {
    enum http_message_type type;
    struct gkab_string start_line[3];
    struct gkab_string header_fields[HF_COUNT];
    struct byte_array content;
};


void http_set_response_status(struct http_message *response, enum http_status_code code, struct gkab_arena *arena) {
    assert(response->type == MT_RESPONSE);
    response->start_line[SL_STATUS_CODE_IDX] = string_from_int((int) code, arena);
    response->start_line[SL_REASON_PHRASE_IDX] = gkab_string_dup_cstring(http_status_code_to_reason_phrase(code), arena);
}


void http_set_header_field(struct http_message *message, enum http_header_field key, struct gkab_string *value) {
    assert(key < HF_COUNT);
    message->header_fields[key] = *value; 
}

void http_init_request(struct http_message *message) {
    message->type = MT_REQUEST;
    message->start_line[0] = gkab_string_empty();
    message->start_line[1] = gkab_string_empty();
    message->start_line[2] = gkab_string_empty();
    for (int i = 0; i < HF_COUNT; i++) {
        message->header_fields[i] = gkab_string_empty();
    }
    message->content.ptr = NULL;
    message->content.len = 0;
    message->content.capacity = 0;
}

void http_init_response(struct http_message *message) {
    message->type = MT_RESPONSE;
    message->start_line[0] = gkab_string_empty();
    message->start_line[1] = gkab_string_empty();
    message->start_line[2] = gkab_string_empty();
    for (int i = 0; i < HF_COUNT; i++) {
        message->header_fields[i] = gkab_string_empty();
    }
    message->content.ptr = NULL;
    message->content.len = 0;
    message->content.capacity = 0;
}

void http_set_start_line(struct http_message *message, struct gkab_string *first, struct gkab_string *second, struct gkab_string *third) {
    message->start_line[0] = *first;
    message->start_line[1] = *second; 
    message->start_line[2] = *third;
}

enum http_status_code http_response_status_code(struct http_message *message) {
    assert(message->type == MT_RESPONSE);
    return (enum http_status_code) atoi(message->start_line[SL_STATUS_CODE_IDX].ptr);
}

void http_deserialize_message_header(struct http_message *message, struct byte_array *arr, struct gkab_arena *arena) {
    int first_space = byte_array_find(arr, &g_space_bytes, 0);
    message->start_line[0] = gkab_string_slice_charstar((char *) arr->ptr, 0, first_space, arena);

    first_space++;
    int second_space = byte_array_find(arr, &g_space_bytes, first_space);
    message->start_line[1] = gkab_string_slice_charstar((char *) arr->ptr, first_space, second_space, arena);

    second_space++;
    int crlf = byte_array_find(arr, &g_single_crlf_bytes, second_space);
    message->start_line[2] = gkab_string_slice_charstar((char *) arr->ptr, second_space, crlf, arena);

    int off = crlf + 2; //skip \r\n

    while (true) {
        for (int i = 0; i < HF_COUNT; i++) {
            struct gkab_string *name = &http_header_field_names[i]; 
            if (off + name->len <= arr->len && memcmp(arr->ptr + off, name->ptr, name->len) == 0) {
                off += name->len + 2; //skip colon and space too
                int crlf = byte_array_find(arr, &g_single_crlf_bytes, off);
                message->header_fields[i] = gkab_string_slice_charstar((char *) arr->ptr, off, crlf, arena);
                break;
            } 
        }
        
        int result = byte_array_find(arr, &g_single_crlf_bytes, off);
        if (result != -1) {
            off = result + 2;
        } else {
            break;
        }
    }
}

size_t http_serialize_start_line(struct byte_array *arr, struct gkab_string start_line[START_LINE_FIELD_COUNT]) {
    off_t start = arr->len;
    byte_array_append_cstring(arr, start_line[RL_METHOD_IDX].ptr);
    byte_array_append_cstring(arr, " ");
    byte_array_append_cstring(arr, start_line[RL_PATH_IDX].ptr);
    byte_array_append_cstring(arr, " ");
    byte_array_append_cstring(arr, start_line[RL_HTTP_VERSION_IDX].ptr);
    byte_array_append_cstring(arr, "\r\n");
    return arr->len - start;
}

size_t http_serialize_header_fields(struct byte_array *arr, struct gkab_string header_fields[HF_COUNT]) {
    off_t start = arr->len;
    for (int i = 0; i < HF_COUNT; i++) {
        if (header_fields[i].len != 0) {
            write_header_field(arr, &http_header_field_names[i], &header_fields[i]);
        }
    }
    return arr->len - start;
}

size_t http_get_message_header_size(struct http_message *message) {
    size_t start_line_size = http_start_line_get_size(message->start_line);
    size_t header_fields_size = http_header_fields_get_size(message->header_fields);
    size_t header_size = start_line_size + header_fields_size + CRLF_SIZE;
    assert(header_size < MAX_HTTP_HEADER_SIZE);
    return header_size;
}

struct byte_array http_serialize_message(struct http_message *message, struct gkab_arena *arena) {
    size_t message_size = http_get_message_header_size(message) + message->content.len;
    struct byte_array buf;
    byte_array_init(&buf, message_size, arena); 

    http_serialize_start_line(&buf, message->start_line);
    http_serialize_header_fields(&buf, message->header_fields);
    byte_array_append_cstring(&buf, "\r\n");
    if (message->content.len != 0) {
        int src_offset = 0;
        byte_array_append_bytes(&buf, &message->content, src_offset, message->content.len);
    }
    assert(buf.len  == message_size);

    return buf;
}

void http_send_message(int sockfd, struct http_message *message, struct gkab_arena *arena) {
    assert(g_initialized);

    struct byte_array buf = http_serialize_message(message, arena);
    net_send(sockfd, &buf);
}

void http_recv_message(int sockfd, struct http_message *message, struct gkab_arena *arena) {
    assert(g_initialized);

    struct byte_array header_buf;
    byte_array_init(&header_buf, MAX_HTTP_HEADER_SIZE, arena);
    size_t content_bytes_read = net_recv_until(sockfd, &header_buf, &g_double_crlf_bytes);

    http_deserialize_message_header(message, &header_buf, arena);

    if (message->header_fields[HF_CONTENT_LENGTH].len == 0) {
        return;
    }

    int content_length = atoi(message->header_fields[HF_CONTENT_LENGTH].ptr);

    //read message content
    byte_array_init(&message->content, content_length, arena);
    byte_array_append_bytes(&message->content, &header_buf, header_buf.len - content_bytes_read, content_bytes_read);
    size_t content_bytes_remaining = content_length - content_bytes_read;
    net_recv(sockfd, &message->content, content_bytes_remaining);
}

void http_init(struct http *http) {
    gkab_arena_init(&http->arena);
    http->route_count = 0;
    http->default_route.name = gkab_string_empty();

    byte_array_init(&g_space_bytes, 1, &http->arena);
    byte_array_append_cstring(&g_space_bytes, " ");
    byte_array_init(&g_single_crlf_bytes, 2, &http->arena);
    byte_array_append_cstring(&g_single_crlf_bytes, "\r\n");
    byte_array_init(&g_double_crlf_bytes, 4, &http->arena);
    byte_array_append_cstring(&g_double_crlf_bytes, "\r\n\r\n");

    g_initialized = true;
}

//void http_listen_and_serve(struct gkab_string *port, int (*handler)(struct http_message*, struct http_message*, struct gkab_arena*)) {
void http_listen_and_serve(struct http* http, struct gkab_string *port) {
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port->ptr, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

            struct gkab_arena arena;
            gkab_arena_init(&arena);

            struct http_message request;
            http_init_request(&request);
            http_recv_message(new_fd, &request, &arena);

            struct http_message response;
            http_init_response(&response);
            response.start_line[SL_HTTP_VERSION_IDX] = gkab_string_dup_cstring("HTTP/1.1", &arena);

            //Change to canonical form (drop trailing slash)
            struct gkab_string path = request.start_line[RL_PATH_IDX];
            if (path.ptr[path.len - 1] == '/') {
                path = gkab_string_slice(&path, 0, path.len - 1, &arena);
                //path = string_init(path.ptr, path.len - 1, &arena);
            }
            //loop through all routes and find matching route handler
            bool handled = false;
            int result = 0;
            for (int i = 0; i < http->route_count; i++) {
                if (gkab_string_cmp(&path, &http->routes[i].name) == 0) {
                    result = http->routes[i].handler(&response, &request, &arena);
                    handled = true;
                    break;
                }
            }
            //if none match, use fallback default route
            if (!handled) {
                result = http->default_route.handler(&response, &request, &arena);
            }

            if (result == 0) {
                http_send_message(new_fd, &response, &arena);
            }
        
            /*
            if (handler(&response, &request, &arena) == 0) {
                http_send_message(new_fd, &response, &arena);
            }
            */

            close(new_fd);
            gkab_arena_free(&arena);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}
}

bool http_write_file(struct http_message *response, const char *path, struct gkab_arena *arena) {
    FILE *f = fopen(path, "rb"); //+1 to skip the slash before path
    if (!f) {
        return false;
    } else {
        fseek(f, 0, SEEK_END);
        size_t len = ftell(f);
        rewind(f);
        byte_array_init(&response->content, len, arena);    
        size_t total_read = fread(response->content.ptr, sizeof(u8), len, f);
        if (total_read != len) {
            return false;
        }
        response->content.len = len;
        struct gkab_string content_length = string_from_int(len, arena);
        http_set_header_field(response, HF_CONTENT_LENGTH, &content_length);
    }
    return true;
}

void http_set_route(struct http *http, const char *route, int (*handler)(struct http_message*, struct http_message*, struct gkab_arena*)) {
    /*
    size_t len = strlen(route);
    //change routes to canonical form (ie, no trailing slashes)
    if (route[len - 1] == '/') {
        len--;
    }

    struct gkab_string s = string_init(route, len, &http->arena);
    */

    struct gkab_string s = gkab_string_dup_cstring(route, &http->arena);
    if (s.cstring[s.len - 1] == '/') {
        s = gkab_string_slice(&s, 0, s.len - 1, &http->arena);
    }

    bool inserted = false;
    for (int i = 0; i < http->route_count; i++) {
        if (gkab_string_cmp(&s, &http->routes[i].name) == 0) {
            http->routes[i].handler = handler;
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        assert(http->route_count < MAX_ROUTES);
        http->routes[http->route_count].name = s; 
        http->routes[http->route_count].handler = handler;
        http->route_count++;
    }
}

void http_set_default_route(struct http *http, int (*handler)(struct http_message*, struct http_message*, struct gkab_arena*)) {
    http->default_route.handler = handler;
}


#endif // HTTP_H
