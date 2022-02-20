#ifndef _ERROR_H_
#define _ERROR_H_

#define VCC_LEX_CPP_CMT_NO_ENDING 0
#define VCC_LEX_NO_CHAR_IN_QUOTE 1

/* error macros use by compiler, not to debug */
#define errorf(fmt, ...)                                                       \
  fprintf(stderr, "[error while " PHASE "]@%s:%d:%d " fmt, fname, line, col,   \
          __VA_ARGS__)

#define errors(str) errorf("%s\n", str)

#endif