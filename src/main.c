#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>

#include "request.h"

#define PORT 5000

struct Server {
    int port;
    int server_fd;
    struct sockaddr_in server_addr;
};

struct Server server;

struct Server* init(int port) {
    if ((server.server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR: socket failed\n");
        return NULL;
    }
    
    int opt = 1;
    if (setsockopt(server.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("ERROR: setsockopt failed\n");
        close(server.server_fd);
        return NULL;
    }

    server.server_addr.sin_family = AF_INET;
    server.server_addr.sin_addr.s_addr = INADDR_ANY;
    server.server_addr.sin_port = htons(port);
    
    if (bind(
        server.server_fd,
        (struct sockaddr *)&server.server_addr,
        sizeof(server.server_addr)) < 0) {
        perror("ERROR: bind failed\n");
        close(server.server_fd);
        return NULL;
    }

    if (listen(server.server_fd, 10) < 0) {
        perror("ERROR: listen failed\n");
        close(server.server_fd);
        return NULL;
    }

    printf("INFO: server successfully started on port %d\n", port);
    return &server;
}

void start(struct Server* server) {
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *clientfd = (int*)malloc(sizeof(int));
        if (!clientfd) continue;

        if ((*clientfd = accept(server->server_fd,
                            (struct sockaddr *)&client_addr,
                            &client_addr_len)) < 0 ) {
            perror("ERROR: accept failed\n");
            free(clientfd);
            continue;
        }

        handle_request(clientfd);

        close(*clientfd);
        free(clientfd);
    }
}

int main(void) {
    struct Server* server = init(PORT);
    if (!server) {
        printf("ERROR: server initializing failed\n");
        return 1;
    }

    start(server);
    
    return 0;
}