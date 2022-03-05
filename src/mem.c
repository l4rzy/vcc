#include "mem.h"

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

void xfree(void *ptr) {
  if (ptr) {
    free(ptr);
  }
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
  b->len = size;
  return b;
}

buf_t *buf_new_from_string(char *str) {
  int len = strlen(str);
  buf_t *b = buf_new(len);
  strncpy(b->s, str, len);
  b->len = len;
  return b;
}

void buf_free(buf_t *buf) {
  if (buf) {
    xfree(buf->s);
    xfree(buf);
  }
}
