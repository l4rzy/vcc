#ifndef _EXPR_H_
#define _EXPR_H_

/* operator precedence
 */
#include "parser.h"
enum {
  OP_MUL,
  OP_ADD,
};

typedef struct _vcc_expr_base_t {
  int arity;

} vcc_expr_base_t;
typedef struct _vcc_expr_t {
  vcc_expr_base_t *base;
  struct _vcc_expr_t *next;
} vcc_expr_t;

vcc_expr_t *vcc_parse_expr(vcc_parser_t *ctx);

#endif