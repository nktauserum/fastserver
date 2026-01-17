#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "request.h"
#include "routes.h"

#define REQUEST_BUFFER_SIZE 1024
struct request_buffer {
    char *buf;
    size_t capacity;
    size_t total_read;
};

void init_buffer(struct request_buffer *r_buf) {
    r_buf->capacity = REQUEST_BUFFER_SIZE * sizeof(char);
    r_buf->buf = (char *)malloc(r_buf->capacity);
}

void clear_buffer(struct request_buffer *r_buf) {
    if (r_buf->buf) free(r_buf->buf);
}

Request parse_request(int *clientfd) {
    Request r;

    struct request_buffer buffer;
    init_buffer(&buffer);

    ssize_t bytes_read = 0;
    if ((bytes_read = recv(*clientfd, buffer.buf + buffer.total_read, buffer.capacity - buffer.total_read - 1, 0)) > 0) {
        buffer.total_read += bytes_read;
        
        if (buffer.total_read >= buffer.capacity - 1) {
            buffer.capacity *= 2;
            char *temp = realloc(buffer.buf, buffer.capacity);
            if (!temp) {
                fprintf(stderr, "ERROR: realloc request_buffer\n");
                goto cleanup;
            }
            buffer.buf = temp;
        }
    }
    
    // buffer.buf now contains raw request data

    char *end_header = strstr(buffer.buf, "\r\n\r\n");
    if (!end_header) {
        end_header = buffer.buf + strlen(buffer.buf);
    }

    ssize_t header_len = end_header - buffer.buf;

    char *request_header = malloc(header_len + 1);
    if (!request_header) {
        fprintf(stderr, "ERROR: malloc request_header\n");
        goto cleanup;
    }

    strncpy(request_header, buffer.buf, header_len);
    request_header[header_len] = '\0';

    char *method = malloc(16 * sizeof(char));
    if (!method) {
        goto cleanup;
    }

    char *path = malloc(256 * sizeof(char));
    if (!path) {
        free(method);
        goto cleanup;
    }

    char *protocol = malloc(256 * sizeof(char));
    if (!protocol) {
        free(method);
        free(path);
        goto cleanup;
    }

    r.method = method;
    r.path = path;
    r.protocol = protocol;
    
    free(request_header);
cleanup:
    clear_buffer(&buffer);
    return r;
}

void handle_request(int *clientfd) {
    Request r = parse_request(clientfd);

    char *status_code = malloc(128 * sizeof(char));
    if (!status_code) {
        return;
    }

    char *response_body = malloc(2048 * sizeof(char));
    if (!response_body) {
        free(status_code);
        return;
    }

    char *mime_type = malloc(256 * sizeof(char));
    if (!mime_type) {
        free(status_code);
        free(response_body);
        return;
    }

    Response w = {
        .mime_type = mime_type,
        .response_body = response_body,
        .status_code = status_code,
    };


    handle_route(&w, r);

    char *response_header = (char*)malloc(2048 * sizeof(char));
    if (!response_header) {
        free(status_code);
        free(mime_type);
        free(response_body);
        return;
    }

    snprintf(response_header, 2048, 
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n\r\n",

        w.status_code, w.mime_type, w.body_size
    );

    send(*clientfd, response_header, strlen(response_header), 0);
    send(*clientfd, w.response_body, w.body_size, 0);

    free(response_header);
    free(w.response_body);
    free(w.mime_type);
    free(w.status_code);
}