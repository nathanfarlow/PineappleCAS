#ifdef COMPILE_PC

#include "yvar.h"

#include <string.h>
#include <malloc.h>

int read_header(header_t *header, FILE *file) {
    char buffer[9] = { 0 };
    fread(buffer, 1, 8, file);

    if (strcmp(buffer, "**TI83F*"))
        return -1;

    fread(buffer, 1, 3, file);

    if (buffer[0] != 26 || buffer[1] != 10 || buffer[2] != 0)
        return -1;

    fread(header->comment, 42, 1, file);

    fread(&header->var_len, 2, 1, file);

    return 0;
}

//todo: more error handling?
int yvar_Read(yvar_t *yvar, FILE *file) {
    int error = 0;
    uint16_t magic;
    uint8_t var_id;

    if (error = read_header(&yvar->header, file) != 0)
        return error;

    fread(&magic, 2, 1, file);

    if (magic != 11 && magic != 13)
        return -1;

    fread(&yvar->len, 2, 1, file);

    fread(&var_id, 1, 1, file);

    if (var_id != 3)
        return -1;

    fread(yvar->name, 1, 8, file);

    fread(&yvar->version, 1, 1, file);

    fread(&yvar->flag, 1, 1, file);

    fread(&yvar->len, 2, 1, file);

    fread(&yvar->yvar_data_len, 2, 1, file);

    yvar->data = malloc(yvar->len);
    fread(yvar->data, yvar->len, 1, file);

    fread(&yvar->header.checksum, 2, 1, file);

    return error;
}

void yvar_Cleanup(yvar_t *yvar) {
    free(yvar->data);
}

#endif