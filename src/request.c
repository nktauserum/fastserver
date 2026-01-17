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
    r_buf->total_read = 0;
}

void clear_buffer(struct request_buffer *r_buf) {
    if (r_buf->buf) free(r_buf->buf);
}

int parse_request(int *clientfd, Request *r) {
    int status = 0;
    struct request_buffer buffer;
    init_buffer(&buffer);

    while (1) {
        // Reallocate the buffer if needed.
        if (buffer.total_read >= buffer.capacity - 1) {
            buffer.capacity *= 2;
            char *temp = realloc(buffer.buf, buffer.capacity);
            if (!temp) {
                fprintf(stderr, "ERROR: realloc request_buffer\n");
                status++;
                goto cleanup;
            }
            buffer.buf = temp;
        }

        ssize_t bytes_read = recv(*clientfd, buffer.buf + buffer.total_read, buffer.capacity - buffer.total_read - 1, 0);
        
        if (bytes_read < 0) {
            perror("recv");
            status++;
            goto cleanup;
        }
        
        if (bytes_read == 0) {
            if (buffer.total_read == 0) {
                status++;
                goto cleanup;
            }
            break;
        }
        
        buffer.total_read += bytes_read;
        
        buffer.buf[buffer.total_read] = '\0';

        if (strstr(buffer.buf, "\r\n\r\n")) break;
    }
    
    // buffer.buf now contains raw request data

    // Select the header. Important for subsequent features.
    char *end_header = strstr(buffer.buf, "\r\n\r\n");
    if (!end_header) {
        end_header = buffer.buf + strlen(buffer.buf);
    }

    ssize_t header_len = end_header - buffer.buf;

    char *request_header = malloc(header_len + 1);
    if (!request_header) {
        fprintf(stderr, "ERROR: malloc request_header\n");
        status++;
        goto cleanup;
    }

    strncpy(request_header, buffer.buf, header_len);
    request_header[header_len] = '\0';

    // Select the first line in the header
    char *end_first_line = strchr(request_header, '\n');
    if (!end_first_line) {
        free(request_header);
        status++;
        goto cleanup;
    }

    ssize_t first_line_len = end_first_line - request_header;

    char *first_line = malloc(++first_line_len);
    if (!request_header) {
        fprintf(stderr, "ERROR: malloc first_line\n");
        free(request_header);
        status++;
        goto cleanup;
    }

    strncpy(first_line, request_header, first_line_len);
    first_line[--first_line_len] = '\0';
    printf("first_line: %s\n", first_line);

    // Allocate request fields
    char *method = malloc(16 * sizeof(char));
    if (!method) {
        fprintf(stderr, "ERROR: malloc request method\n");
        status++;
        goto cleanup;
    }

    char *path = malloc(2048 * sizeof(char));
    if (!path) {
        fprintf(stderr, "ERROR: malloc request path\n");
        free(method);
        status++;
        goto cleanup;
    }

    char *protocol = malloc(256 * sizeof(char));
    if (!protocol) {
        fprintf(stderr, "ERROR: malloc request protocol\n");
        free(method);
        free(path);
        status++;
        goto cleanup;
    }

    // Select the required information
    int parsed_count; 
    if ((parsed_count = sscanf(first_line, "%s %s %s\r", method, path, protocol)) != 3) {
        printf("ERROR: parsing the line \"%s\"\n", first_line);
        free(method);
        free(path);
        free(protocol);
        free(first_line);
        status++;
        goto cleanup;
    }

    r->method = method;
    r->path = path;
    r->protocol = protocol;
    
cleanup:
    if (request_header) free(request_header);
    if (first_line) free(first_line);
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
    if (r.method) free(r.method);
    if (r.path) free(r.path);
    if (r.protocol) free(r.protocol);
}