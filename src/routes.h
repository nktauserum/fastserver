#ifndef ROUTES_H
#define ROUTES_H

#include "request.h"

typedef void (*Handler)(Response *w, Request r);

typedef struct {
    const char* route;
    Handler handler;
} Route;

void handle_route(Response *w, Request r);

#endif // ROUTES_H