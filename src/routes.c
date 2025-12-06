#include "routes.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define INDEX_PATH "static/index.html"
#define ABOUT_PATH "static/about.html"
#define NOT_FOUND_PATH "static/404.html"
#define FAVICON_PATH "static/favicon.png"

void index_handler(int* clientfd) {
    char *content = load_txt_file(INDEX_PATH);
    if (content) {
        char* response = (char*)malloc(2048*2*sizeof(char));
        char *header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n", strlen(content) + 1);
        
        snprintf(response, 2048*2, 
        "%s\r\n%s\n", header, content);

        send(*clientfd, response, strlen(response), 0);
        
        free(header);
        free(content);
        free(response);
    }
}

void about_handler(int* clientfd) {
    char *content = load_txt_file(ABOUT_PATH);
    if (content) {
        char* response = (char*)malloc(2048*2*sizeof(char));
        char *header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %zu\r\n", strlen(content) + 1);
        
        snprintf(response, 2048*2, 
        "%s\r\n%s\n", header, content);

        send(*clientfd, response, strlen(response), 0);
        
        free(header);
        free(content);
        free(response);
    }
}

void not_found_handler(int* clientfd) {
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

        send(*clientfd, response, strlen(response), 0);
        
        free(header);
        free(content);
        free(response);
    }
}

void image_handler(int* clientfd) {
    const char* image_path = "static/image.jpg"; 

    ImageFile* img = load_image_file(image_path);
    if (img) {
        char* header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n", img->type, img->size);
        
        send(*clientfd, header, strlen(header), 0);
        send(*clientfd, img->data, img->size, 0);
        
        free(header);
        free(img->data);
        free(img);
    }
}

void favicon_handler(int* clientfd) {
    const char* image_path = FAVICON_PATH; 

    ImageFile* img = load_image_file(image_path);
    if (img) {
        char* header = (char *)malloc(2048 * sizeof(char));
        snprintf(header, 2048, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n\r\n", img->type, img->size);
        
        send(*clientfd, header, strlen(header), 0);
        send(*clientfd, img->data, img->size, 0);
        
        free(header);
        free(img->data);
        free(img);
    }
}

Route router[] = {
    {.route = "GET /", .handler = index_handler},
    {.route = "GET /about", .handler = about_handler},
    {.route = "GET /img", .handler = image_handler},
    {.route = "GET /favicon.ico", .handler = favicon_handler},
    {.route = NULL, .handler = not_found_handler}
};

void handle_route(const char* request, int* clientfd) {
    for (int i = 0; router[i].route != NULL; i++) {
        if (strcmp(request, router[i].route) == 0) {
            router[i].handler(clientfd);
            return;
        }
    }

    not_found_handler(clientfd);
}