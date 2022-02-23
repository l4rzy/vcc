#ifndef _STMT_H_
#define _STMT_H_

#include "expr.h"

enum VCC_STMT_TYPE {
  STMT_TYPE_IF,
  STMT_TYPE_WHILE,
  STMT_TYPE_RETURN,
  STMT_TYPE_DECL
};

typedef struct _vcc_closure_t {

} vcc_closure_t;

typedef struct _vcc_func_t {
  int arity;
  char *name;
  vcc_closure_t *body;
} vcc_func_t;

typedef struct _vcc_stmt_t {
  int type;
  vcc_expr_t *condition;
  vcc_closure_t *body;
  vcc_expr_t *expr;

} vcc_stmt_t;
#endif