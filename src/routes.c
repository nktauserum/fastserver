#include "routes.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define INDEX_PATH "static/index.html"
#define ABOUT_PATH "static/about.html"
#define NOT_FOUND_PATH "static/404.html"
#define FAVICON_PATH "static/image/favicon.png"

struct Request {
    char *path, *method, *protocol;
};

struct Response {
    char *status_code;
    char *mime_type;

    char *response_body;
    size_t body_size;
};

void index_handler(Response *w, Request r) {
    char *content = load_txt_file(INDEX_PATH);
    if (content) {
        ssize_t content_len = strlen(content);
        snprintf(w->response_body, content_len, "%s", content);
        snprintf(w->mime_type, 256, "%s", "text/html");
        snprintf(w->status_code, 256, "%s", "200 OK");
        w->body_size = content_len;
        free(content);
    } else {
        printf("error loading file\n");
    }
}

void not_found_handler(Response *w, Request r) {
    char *content = load_txt_file(NOT_FOUND_PATH);
    if (content) {
        ssize_t content_len = strlen(content);
        snprintf(w->response_body, content_len, "%s", content);
        snprintf(w->mime_type, 256, "%s", "text/html");
        snprintf(w->status_code, 256, "%s", "404 Not Found");
        w->body_size = content_len;
        free(content);
    }
}

Route router[] = {
    {.route = "/", .handler = index_handler},
    {.route = NULL, .handler = not_found_handler}
};

void handle_route(Response *w, Request r) {
    for (int i = 0; router[i].route != NULL; i++) {
        if (strcmp(r.path, router[i].route) == 0) {
            router[i].handler(w, r);
            return;
        }
    }

    printf("Not found: %s\n", r.path);
    not_found_handler(w,r);
}