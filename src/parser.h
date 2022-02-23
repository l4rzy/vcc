#ifndef _PARSER_H_
#define _PARSER_H_

#include "error.h"
#include "lexer.h"

enum {
  NODE_TYPE_FUNC = 0,
  NODE_TYPE_STMT_IF,
  NODE_TYPE_STMT_WHILE,
  NODE_TYPE_STMT_RETURN,
  NODE_TYPE_STMT_DECLARE
};

typedef struct _vcc_parser_t {
  vtoken_t *current;
  vtoken_t *next;

  int error;
} vcc_parser_t;

void vcc_parser_init();
void vcc_parser_finish();
int vcc_parser_eof();
void vcc_parse();

#endif