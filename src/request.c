#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "request.h"
#include "routes.h"

#define REQUEST_BUFFER_SIZE 1024

void init_buffer(request_buffer *r_buf) {
    r_buf->capacity = REQUEST_BUFFER_SIZE * sizeof(char);
    r_buf->buf = (char *)malloc(r_buf->capacity);
    r_buf->total_read = 0;
}

void clear_buffer(request_buffer *r_buf) {
    if (r_buf->buf) free(r_buf->buf);
}

int parse_request(int *clientfd, Request *r) {
    int status = 0;
    request_buffer buffer;
    init_buffer(&buffer);

    while (1) {
        // Reallocate the buffer if needed.
        if (buffer.total_read >= buffer.capacity - 1) {
            buffer.capacity *= 2;
            char *temp = realloc(buffer.buf, buffer.capacity);
            if (!temp) {
                fprintf(stderr, "ERROR: realloc request_buffer\n");
                status++;
                goto error;
            }
            buffer.buf = temp;
        }

        ssize_t bytes_read = recv(*clientfd, buffer.buf + buffer.total_read, buffer.capacity - buffer.total_read - 1, 0);
        
        if (bytes_read < 0) {
            perror("recv");
            status++;
            goto error;
        }
        
        if (bytes_read == 0) {
            if (buffer.total_read == 0) {
                status++;
                goto error;
            }
            break;
        }
        
        buffer.total_read += bytes_read;
        
        buffer.buf[buffer.total_read] = '\0';

        if (strstr(buffer.buf, "\r\n\r\n")) break;
    }
    
    // buffer.buf now contains raw request data
    r->buf = buffer.buf;

    const char *p = buffer.buf;
    const char *end = buffer.buf + buffer.total_read;
    int state = s_start;

    while (p < end) {
        const char *c = p++;
        switch (state)
        {
        case s_start:
            state = s_method;
            r->method = c;
            break;

        case s_method:
            if (*c == ' ') {
                state = s_path;
                r->path = c+1;
                r->method_len = c - r->method;
            }
            break;
            
        case s_path:
            if (*c == ' ') {
                state = s_protocol;
                r->protocol = c+1;
                r->path_len = c - r->path;
            }
            break;
        
        case s_protocol:
            if (*c == '\r') {
                state = s_done;
                r->protocol_len = c - r->protocol;
            }
            break;
        
        case s_done:
            break;
        }
    }

    return status;
    
error:
    clear_buffer(&buffer);
    return status;
}

void handle_request(int *clientfd) {
    Request r = {0};
    int s;
    if ((s = parse_request(clientfd, &r)) != 0) {
        fprintf(stderr, "ERROR: parse request\n");
        return;
    }

    char *status_code = malloc(128 * sizeof(char));
    if (!status_code) goto cleanup;

    char *response_body = malloc(2048 * sizeof(char));
    if (!response_body) goto cleanup;

    char *mime_type = malloc(256 * sizeof(char));
    if (!mime_type) goto cleanup;

    Response w = {
        .mime_type = mime_type,
        .response_body = response_body,
        .status_code = status_code,
    };


    handle_route(&w, r);

    char *response_header = (char*)malloc(2048 * sizeof(char));
    if (!response_header) goto cleanup;

    snprintf(response_header, 2048, 
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Connection: close\r\n"
        "Content-Length: %zu\r\n\r\n",

        w.status_code, w.mime_type, w.body_size
    );

    send(*clientfd, response_header, strlen(response_header), 0);
    send(*clientfd, w.response_body, w.body_size, 0);
    
cleanup:    
    if (status_code) free(status_code);
    if (response_body) free(response_body);
    if (mime_type) free(mime_type);
    if (response_header) free(response_header);
}