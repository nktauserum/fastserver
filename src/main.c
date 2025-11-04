#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <regex.h>

#define PORT 5000
#define BUFFER_SIZE 1024


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

struct Server {
    int port;
    int server_fd;
    struct sockaddr_in server_addr;
};

struct Server server;
regex_t regex;

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

void handle_request(int* clientfd) {
    char *request_buffer = (char *)malloc(1024 * sizeof(char));
    ssize_t bytes_received = recv(*clientfd, request_buffer, 1024, 0);    
    if (bytes_received > 0) {
        char* response = (char*)malloc(2048*2*sizeof(char));
        if (!response) goto cleanup;

        regcomp(&regex, "GET /([^ ]*)*", REG_EXTENDED);
        regmatch_t ms[2];

        if (regexec(&regex, request_buffer, 2, ms, 0) < 0) {
            printf("ERROR: regexp executing failed");
            goto cleanup;
        }
        request_buffer[ms[1].rm_eo] = '\0';

        printf("INFO: %s\n", request_buffer);             

        char* content;
        char* status;
        if (strcmp(request_buffer, "GET /") == 0) {
            content = read_file("index.html");
            status = "200 OK";
        } else if (strcmp(request_buffer, "GET /about") == 0) {
            content = read_file("about.html");
            status = "200 OK";
        } else {
            content = read_file("404.html");
            status = "404 Not Found";
        }

        char* header = (char*)malloc(2048*sizeof(char));
        if (!header) goto cleanup;
        snprintf(header, 2048, 
        "HTTP/1.1 %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n", status, strlen(content)+1); // Учитываем \0-терминатор
        
        snprintf(response, 2048*2, 
        "%s\r\n%s\n", header, content);

        send(*clientfd, response, strlen(response), 0);
        
        free(header);
        free(response);
        free((void*)content);
    }

cleanup:
    close(*clientfd);
    free(clientfd);
    free(request_buffer);
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
    }
}


int main() {
    struct Server* server = init(PORT);
    if (!server) {
        printf("ERROR: server initializing failed\n");
        return 1;
    }

    start(server);
}