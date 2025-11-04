#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>

char* load_txt_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = (char*)malloc(len + 1);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, len, f);

    data[len] = '\0';

    fclose(f);
    return data;
}

#endif // FILE_H