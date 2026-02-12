#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>

enum {
    s_start,
    s_path,
    s_method,
    s_protocol,
    s_done
};

typedef struct {
    char *buf;
    size_t capacity;
    size_t total_read;
} request_buffer;

typedef struct {
    char *buf;
    char *path;         size_t path_len;
    char *method;       size_t method_len;
    char *protocol;     size_t protocol_len;
} Request;

typedef struct {
    char *status_code;
    char *response_body;
    char *mime_type;
    size_t body_size;
} Response;

void handle_request(int *);

#endif // REQUEST_H