#include "utils.h"

long file_len(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long result = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return result;
}

void *xalloc(size_t size) {
    void *ret = malloc(size);
    if (!ret) {
        fatals("could not allocate memory\n");
    }
    bzero(ret, size);
    return ret;
}

buf_t *buf_new(int len) {
    buf_t *b = xalloc(sizeof(buf_t));
    b->s = xalloc(len + 1);
    b->len = 0;
    return b;
}

buf_t *buf_new_from_file(FILE *fp) {
    long len = file_len(fp);
    buf_t *b = xalloc(sizeof(buf_t));
    b->s = xalloc(len + 1);
    fread(b->s, len, 1, fp);
    b->len = len;
    return b;
}

buf_t *buf_new_from_mem(char *mem, size_t size) {
    buf_t *b = xalloc(sizeof(buf_t));
    b->s = xalloc(size + 1);
    memcpy(b->s, mem, size);
}

void buf_free(buf_t *buf) {
    free(buf->s);
    free(buf);
}
