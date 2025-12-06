#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char* data;
    unsigned char* type;
    size_t size;
} ImageFile;

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

const char* get_mime_type(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";

    return "application/octet-stream";
}

ImageFile* load_image_file(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    unsigned char* data = (unsigned char*)malloc(len);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, len, f);

    ImageFile* img = (ImageFile*)malloc(sizeof(ImageFile));
    img->data = data;
    img->type = (unsigned char*)get_mime_type(filename);
    img->size = len;

    fclose(f);
    return img;
}

#endif // FILE_H