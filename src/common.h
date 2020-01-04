#ifndef _COMMON_H_
#define _COMMON_H_

#define ERR_MAX_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef ENABLE_DEBUG
#define logf(fmt, ...) \
    do { \
        fprintf(stderr, "[LOG][\033[0;32m%s\033[0m:%d:\033[0;34m%s()\033[0m] "fmt, __FILE__, __LINE__,\
                __func__, __VA_ARGS__);\
    } while (0)

#define logs(str) \
    logf("%s", str)

#else
#define logf(fmt, ...)
#define logs(str)
#endif

/* error macros use by compiler, not to debug */
#define errorf(fmt, ...) \
    fprintf(stderr, "[" PHASE "]@%s:%d "fmt,                                                       \
             fname, line, __VA_ARGS__)

#define errors(str) \
    errorf("%s\n", str)

/* fatal due to compiler bug */
#define fatalf(fmt, ...) \
    do { \
        fprintf(stderr, "[FATAL][%s:%d:%s()] "fmt, __FILE__, __LINE__, \
                __func__, __VA_ARGS__);\
        exit(-1); \
    } while (0)

#define fatals(str) \
    fatalf("%s", str)

#endif
