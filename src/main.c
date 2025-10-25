#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

char* read_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = malloc(len + 1);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, len, f);

    data[len] = '\0';

    fclose(f);
    return data;
}

void handle_request(int* clientfd) {
    char *buffer = (char *)malloc(1024 * sizeof(char));
    ssize_t bytes_received = recv(*clientfd, buffer, 1024, 0);    
    if (bytes_received > 0) {
        char* response = (char*)malloc(2048*2*sizeof(char));
        if (!response) goto cleanup;

        const char* content = read_file("index.html");
        if (!content) content = "<html><body>File not found</body></html>";

        char* header = (char*)malloc(2048*sizeof(char));
        if (!header) goto cleanup;
        snprintf(header, 2048, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n", strlen(content));
        
        snprintf(response, 2048*2, 
        "%s\r\n%s\n", header, content);

        send(*clientfd, response, strlen(response), 0);
        
        free(header);
        free(response);
        free((void*)content);
    }

cleanup:
    close(*clientfd);
    free(buffer);
    free(clientfd);
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR: socket failed\n");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("ERROR: setsockopt failed\n");
        close(server_fd);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5000);

    if (bind(
        server_fd,
        (struct sockaddr *)&server_addr,
        sizeof(server_addr)) < 0) {
        perror("ERROR: bind failed\n");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("ERROR: listen failed\n");
        close(server_fd);
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *clientfd = (int*)malloc(sizeof(int));
        if (!clientfd) continue;

        if ((*clientfd = accept(server_fd,
                            (struct sockaddr *)&client_addr,
                            &client_addr_len)) < 0 ) {
            perror("ERROR: accept failed\n");
            free(clientfd);
            return 1;
        }

        handle_request(clientfd);
    }
}