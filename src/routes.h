#ifndef ROUTES_H
#define ROUTES_H

typedef struct {
    int* clientfd;
    const char* path;
} Request;

typedef void (*Handler)(Request req);

typedef struct {
    const char* route;
    int dynamic;
    Handler handler;
} Route;

void handle_route(const char* response, int* clientfd);

#endif // ROUTES_H