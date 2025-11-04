#include "routes.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void index_handler(int* clientfd) {
    char *content = load_txt_file("index.html");
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
    char *content = load_txt_file("about.html");
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
    char *content = load_txt_file("404.html");
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

Route router[] = {
    {.route = "GET /", .handler = index_handler},
    {.route = "GET /about", .handler = about_handler},
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