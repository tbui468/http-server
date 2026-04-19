#ifndef NET_H
#define NET_H

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdbool.h>

#include "gkab_string.h"
#include "byte_array.h"

#define BACKLOG 10	 // how many pending connections queue will hold

void sigchld_handler(int s)
{
    s = s; //quiet warnings
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

bool net_tcp_connection(struct gkab_string *host, struct gkab_string *port, int *sockfd) {
	struct addrinfo hints, *servinfo, *p;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

    int rv = getaddrinfo(host->ptr, port->ptr, &hints, &servinfo);

    if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return false;
    }

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((*sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(*sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return false;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);

	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    return true;
}

void net_send(int sockfd, struct byte_array *arr) {
    int total_sent = 0;
    while (total_sent < arr->len) {
        int sent = send(sockfd, arr->ptr + total_sent, arr->len - total_sent, 0);
        if (sent == -1) {
            perror("send");
            exit(1);
        } 
        total_sent += sent;
    }
}

//calls recv until the entire delimiter is read into arr
//returns number of extra bytes read _after_ delimiter
size_t net_recv_until(int sockfd, struct byte_array *arr, struct byte_array *delimiter) {
    while (true) {
        int numbytes = recv(sockfd, arr->ptr + arr->len, arr->capacity - arr->len, 0);
        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }

        arr->len += numbytes;
        int idx = byte_array_find(arr, delimiter, 0);
        if (idx != -1) {
            return arr->len - idx - 4; //remove 2 crlfs
        }
    }
}

void net_recv(int sockfd, struct byte_array *arr, size_t count) {
    int total_count = 0;
    while (total_count < count) {
        int numbytes = recv(sockfd, arr->ptr + arr->len, count - total_count, 0);
        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }

        arr->len += numbytes;
        total_count += numbytes;
    }
}

bool net_get_url_scheme(struct gkab_string *url, struct gkab_string *result, struct gkab_arena *arena) {
    int idx = gkab_string_find_cstring(url, "http://", 0);
    if (idx != -1) {
        *result = gkab_string_slice(url, 0, 4, arena);
        return true;
    }
    return false;
}


bool net_get_url_host(struct gkab_string *url, struct gkab_string *result, struct gkab_arena *arena) {
    int start_idx = gkab_string_find_cstring(url, "://", 0);
    if (start_idx == -1) {
        return false;
    }
    start_idx += 3;

    int port_idx = gkab_string_find_cstring(url, ":", start_idx);
    int path_idx = gkab_string_find_cstring(url, "/", start_idx);
    int end_idx;
    if (port_idx == -1 && path_idx == -1) {
        end_idx = url->len;
    } else if ((port_idx == -1 && path_idx != -1) || 
               (port_idx != -1 && path_idx != -1 && path_idx < port_idx)) {
        end_idx = path_idx;
    } else if ((path_idx == -1 && port_idx != -1) ||
               (port_idx != -1 && path_idx != -1 && port_idx < path_idx)) {
        end_idx = port_idx;
    } else {
        assert(false);
    }

    *result = gkab_string_slice_charstar(url->ptr, start_idx, end_idx, arena);
    
    return true;
}

bool net_get_url_port(struct gkab_string *url, struct gkab_string *result, struct gkab_arena *arena) {
    int start_idx = gkab_string_find_cstring(url, ":", 0);
    int port_idx = gkab_string_find_cstring(url, ":", start_idx + 1);

    if (port_idx == -1) {
        return false;
    }

    int beg = port_idx + 1;
    int end = beg;
    while (true) {
        char c = *(url->ptr + end);
        if (c >= '0' && c <= '9') {
            end++;
            continue;
        }
        break;
    }

    if (end == beg) {
        return false;
    }

    *result = gkab_string_slice_charstar(url->ptr, beg, end, arena);

    return true;
}


bool net_get_url_path(struct gkab_string *url, struct gkab_string *result, struct gkab_arena *arena) {
    int start_idx = gkab_string_find_cstring(url, "://", 0);
    int filepath_idx = gkab_string_find_cstring(url, "/", start_idx + 3);

    if (filepath_idx == -1) {
        return false;
    }

    *result = gkab_string_slice_charstar(url->ptr, filepath_idx, url->len, arena);
    
    return true;
}



#endif //NET_H
