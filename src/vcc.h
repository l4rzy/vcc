#ifndef _COMMON_H_
#define _COMMON_H_

#define ERR_MAX_SIZE 1024

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_DEBUG
#define logf(fmt, ...)                                                         \
  do {                                                                         \
    fprintf(stderr, "[%s][\033[0;32m%s\033[0m:%d:\033[0;34m%s()\033[0m] " fmt, \
            PHASE, __FILE__, __LINE__, __func__, __VA_ARGS__);                 \
  } while (0)

#define logs(str) logf("%s", str)

#else
#define logf(fmt, ...)
#define logs(str)
#endif

/* fatal due to compiler or system problems */
#define fatalf(fmt, ...)                                                       \
  do {                                                                         \
    fprintf(stderr, "[FATAL][%s:%d:%s()] " fmt, __FILE__, __LINE__, __func__,  \
            __VA_ARGS__);                                                      \
    exit(-1);                                                                  \
  } while (0)

#define fatals(str) fatalf("%s", str)

#endif
