
#ifndef _UTILS_H_
#define _UTILS_H_

#include "common.h"
#include <errno.h>

/* file io functions */
long file_len(FILE *);

/* memory allocation functions */
void *xalloc(size_t size);

/* common types */
typedef struct _buf_t {
    char *s;
    int len;
} buf_t;

buf_t *buf_new(int);
buf_t *buf_new_from_file(FILE *);
buf_t *buf_new_from_mem(char *, size_t);
void buf_free(buf_t *);

#endif
