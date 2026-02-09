#include <stdio.h>
#include <stdlib.h>

char *mason_read_file_to_string(const char *path, size_t *out_len) {
    FILE *file = NULL;
    char *buffer = NULL;
    long size = -1;
    size_t read_bytes = 0;

    file = fopen(path, "rb");
    if (!file) {
        perror("fopen");
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("fseek");
        goto cleanup;
    }

    size = ftell(file);
    if (size < 0) {
        perror("ftell");
        goto cleanup;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("fseek");
        goto cleanup;
    }

    buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        perror("malloc");
        goto cleanup;
    }

    read_bytes = fread(buffer, 1, (size_t)size, file);
    if (read_bytes != (size_t)size) {
        perror("fread");
        goto cleanup;
    }

    buffer[size] = '\0';
    if (out_len) {
        *out_len = (size_t)size;
    }

cleanup:
    if (file)
        fclose(file);
    if (buffer && (size < 0 || read_bytes != (size_t)size)) {
        free(buffer);
        buffer = NULL;
    }
    return buffer;
}
