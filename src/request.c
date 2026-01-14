#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "request.h"
#include "routes.h"

regex_t regex;

#define REQUEST_BUFFER_SIZE 1024
struct request_buffer {
    char *buf;
    size_t capacity;
};

void init_buffer(struct request_buffer *r_buf) {
    r_buf->capacity = REQUEST_BUFFER_SIZE * sizeof(char);
    r_buf->buf = (char *)malloc(r_buf->capacity);
}

void clear_buffer(struct request_buffer *r_buf) {
    free(r_buf->buf);
}

void handle_request(int *clientfd) {
    struct request_buffer buffer;
    init_buffer(&buffer);
    
    ssize_t bytes_received = recv(*clientfd, buffer.buf, 1024, 0);    
    if (bytes_received > 0) {
        regcomp(&regex, "GET /([^ ]*)*", REG_EXTENDED);
        regmatch_t ms[2];

        if (regexec(&regex, buffer.buf, 2, ms, 0) < 0) {
            printf("ERROR: regexp executing failed");
            goto cleanup;
        }
        buffer.buf[ms[1].rm_eo] = '\0';

        printf("INFO: %s\n", buffer.buf);

        char *status_code = malloc(128 * sizeof(char));
        if (!status_code) {
            goto cleanup;
        }

        char *response_body = malloc(2048 * sizeof(char));
        if (!response_body) {
            free(status_code);
            goto cleanup;
        }

        char *mime_type = malloc(256 * sizeof(char));
        if (!mime_type) {
            free(status_code);
            free(response_body);
            goto cleanup;
        }

        Response w = {
            .mime_type = mime_type,
            .response_body = response_body,
            .status_code = status_code,
        };

        Request r = {.path = buffer.buf};

        handle_route(&w, r);

        char *header = (char*)malloc(2048 * sizeof(char));
        if (!header) {
            free(status_code);
            free(mime_type);
            free(response_body);
            goto cleanup;
        }

        snprintf(header, 2048, 
            "HTTP/1.1 %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n",

            w.status_code, w.mime_type, w.body_size
        );

        send(*clientfd, header, strlen(header), 0);
        send(*clientfd, w.response_body, w.body_size, 0);

        free(header);
        free(w.response_body);
        free(w.mime_type);
        free(w.status_code);
    }

cleanup:
    clear_buffer(&buffer);
}