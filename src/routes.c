#include "routes.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define INDEX_PATH "static/index.html"
#define ABOUT_PATH "static/about.html"
#define NOT_FOUND_PATH "static/404.html"
#define FAVICON_PATH "static/image/favicon.png"

#define status_ok(w) snprintf(w->status_code, 256, "%s", "200 OK")
#define status_not_found(w) snprintf(w->status_code, 256, "%s", "404 Not Found")
#define status_internal_server_error(w) snprintf(w->status_code, 256, "%s", "500 Internal Server Error")
#define mime_html(w) snprintf(w->mime_type, 256, "%s", "text/html")

void return_html(Response *w, char *content) {
    ssize_t content_len = strlen(content);
    snprintf(w->response_body, 2048, "%s", content);
    snprintf(w->mime_type, 256, "%s", "text/html");
    w->body_size = content_len;
}

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
        return_html(w, content);
        status_ok(w);
        free(content);
    } else {
        printf("ERROR: could not load index.html\n");
        return_html(w, "<html><head><title>500 Internal Server Error</title></head><body>500 Internal Server Error</body></html>");
        status_internal_server_error(w);
    }
}

void not_found_handler(Response *w, Request r) {
    char *content = load_txt_file(NOT_FOUND_PATH);
    if (content) {
        return_html(w, content);
        status_not_found(w);
        free(content);
    } else {
        return_html(w, "<html><head><title>404 Not Found</title></head><body>404 Not Found</body></html>");
        status_not_found(w);
    }
}

Route router[] = {
    {.method = "GET", .route = "/", .handler = index_handler},
    {.route = NULL, .handler = not_found_handler}
};

void handle_route(Response *w, Request r) {
    for (int i = 0; router[i].route != NULL; i++) {
        if ((strncmp(r.path, router[i].route, r.path_len) == 0) && (strncmp(r.method, router[i].method, r.method_len) == 0)) {
            router[i].handler(w, r);
            return;
        }
    }

    printf("Not found: %s\n", strndup(r.path, r.path_len));
    not_found_handler(w,r);
}