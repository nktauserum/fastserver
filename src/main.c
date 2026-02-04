#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "request.h"

#define PORT 5000

struct {
    int port;
    int server_fd;
    struct sockaddr_in server_addr;
} server;

int main(void) {
   if ((server.server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR: socket failed\n");
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(server.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("ERROR: setsockopt failed\n");
        close(server.server_fd);
        return 1;
    }

    server.server_addr.sin_family = AF_INET;
    server.server_addr.sin_addr.s_addr = INADDR_ANY;
    server.server_addr.sin_port = htons(PORT);
    
    if (bind(
        server.server_fd,
        (struct sockaddr *)&server.server_addr,
        sizeof(server.server_addr)) < 0) {
        perror("ERROR: bind failed\n");
        close(server.server_fd);
        return 1;
    }

    if (listen(server.server_fd, 10) < 0) {
        perror("ERROR: listen failed\n");
        close(server.server_fd);
        return 1;
    }

    printf("INFO: server successfully started on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *clientfd = (int*)malloc(sizeof(int));
        if (!clientfd) continue;

        if ((*clientfd = accept(server.server_fd,
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
    
    return 0;
}