#ifndef ROUTES_H
#define ROUTES_H

typedef void (*Handler)(int* clientfd);

typedef struct {
    const char* route;
    Handler handler;
} Route;

void handle_route(const char* response, int* clientfd);

#endif // ROUTES_H