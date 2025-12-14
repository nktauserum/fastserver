#include "routes.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define INDEX_PATH "static/index.html"
#define ABOUT_PATH "static/about.html"
#define NOT_FOUND_PATH "static/404.html"
#define FAVICON_PATH "static/image/favicon.png"

#define send_html(content) do {                                     \
        char* response = (char*)malloc(2048*2*sizeof(char));        \
        char *header = (char *)malloc(2048 * sizeof(char));         \
        snprintf(header, 2048,                                      \
            "HTTP/1.1 200 OK\r\n"                                   \
            "Content-Type: text/html\r\n"                           \
            "Content-Length: %zu\r\n", strlen(content) + 1);        \
        snprintf(response, 2048*2,                                  \
        "%s\r\n%s\n", header, content);                             \
        send(*req.clientfd, response, strlen(response), 0);         \
        free(header);                                               \
        free(content);                                              \
        free(response);                                             \
    } while (0)

struct Request {
    int* clientfd;
    const char* path;
};

void index_handler(Request req) {
    char *content = load_txt_file(INDEX_PATH);
    if (content) send_html(content);
}


void about_handler(Request req) {
    char *content = load_txt_file(ABOUT_PATH);
    if (content) send_html(content);
}

void not_found_handler(Request req) {
    char *content = load_txt_file(NOT_FOUND_PATH);
    if (content) {
        char* response = (char*)malloc(2048*2*sizeof(char));
        char *header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n", strlen(content) + 1);
        
        snprintf(response, 2048*2, 
        "%s\r\n%s\n", header, content);

        send(*req.clientfd, response, strlen(response), 0);
        
        free(header);
        free(content);
        free(response);
    }
}

void static_handler(Request req) {
    char path[256];
    snprintf(path, 256, "static/%s", req.path);

    File* img = load_file(path);
    if (img) {
        char* header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n", img->type, img->size);
        
        send(*req.clientfd, header, strlen(header), 0);
        send(*req.clientfd, img->data, img->size, 0);
        
        free(header);
        free(img->data);
        free(img);
    } else {
        not_found_handler(req);
    }
}

void favicon_handler(Request req) {
    File* img = load_file(FAVICON_PATH);
    if (img) {
        char* header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n", img->type, img->size);
        
        send(*req.clientfd, header, strlen(header), 0);
        send(*req.clientfd, img->data, img->size, 0);
        
        free(header);
        free(img->data);
        free(img);
    } else {
        not_found_handler(req);
    }
}

Route router[] = {
    {.route = "GET /", .handler = index_handler, .dynamic=0},
    {.route = "GET /about", .handler = about_handler, .dynamic=0},
    {.route = "GET /static/%255s", .handler = static_handler, .dynamic=1},
    {.route = "GET /favicon.ico", .handler = favicon_handler, .dynamic=0},
    {.route = NULL, .handler = not_found_handler, .dynamic=0}
};

void handle_route(const char* request, int* clientfd) {
    Request req = {.clientfd = clientfd, .path = request};
    for (int i = 0; router[i].route != NULL; i++) {

        if (router[i].dynamic > 0) {
            char path[256];
            if (sscanf(request, router[i].route, path) == 1) {
                req.path = path;
                router[i].handler(req);
                return;
            }
            continue;
        }
        if (strcmp(request, router[i].route) == 0) {
            router[i].handler(req);
            return;
        }
    }

    not_found_handler(req);
}