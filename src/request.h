#ifndef REQUEST_H
#define REQUEST_H

#include <stdlib.h>

typedef struct {
    char *path;
} Request;

typedef struct {
    char *status_code;
    char *response_body;
    char *mime_type;
    size_t body_size;
} Response;

void handle_request(int *);

#endif // REQUEST_H