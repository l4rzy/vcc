#ifndef _PARSER_H_
#define _PARSER_H_

#include "error.h"
#include "lexer.h"
#include "mem.h"

enum {
  VCC_NODE_FUNC = 0,
  VCC_NODE_STMT,
};

enum {
  PREC_EXPR = 0,
  PREC_EQUALITY,
  PREC_COMPARISON,
  PREC_TERM,
  PREC_FACTOR,
  PREC_UNARY,
  PREC_PRIMARY
};

enum { STMT_TYPE_IF, STMT_TYPE_WHILE, STMT_TYPE_RETURN, STMT_TYPE_DECL };

enum { EXPR_OP_MUL = 0, EXPR_OP_DIV, EXPR_OP_ADD, EXPR_OP_SUB };

enum { VCC_PARSER_ERR_NONE = 0, VCC_PARSER_ERR_STMT, VCC_PARSER_ERR_EXPR };

extern const char *node_types[];
extern const char *stmt_types[];

/* ========= EXPRESSIONS ========== */
typedef struct _vcc_expr_t {
  int arity; // numbers of argument
  int prec;  // precedence
  int opr;   // operator
  struct _vcc_expr_t *lhs;
  struct _vcc_expr_t *rhs;
  // atomic expression e.g. number, function call
  union {
    int number;
    void *func_call;
  } atomic;
  struct _vcc_expr_t *next;
} vcc_expr_t;

vcc_expr_t *vcc_expr_new();
vcc_expr_t *vcc_expr_new_binary(vcc_expr_t *lhs, int opr, vcc_expr_t *rhs);
vcc_expr_t *vcc_expr_new_unary(int opr, vcc_expr_t *rhs);
vcc_expr_t *vcc_expr_new_atomic_int(int val);
vcc_expr_t *vcc_expr_new_atomic_null();

vcc_expr_t *vcc_expr_parse();
vcc_expr_t *vcc_expr_parse_equality();
vcc_expr_t *vcc_expr_parse_comparison();
vcc_expr_t *vcc_expr_parse_term();
vcc_expr_t *vcc_expr_parse_factor();
vcc_expr_t *vcc_expr_parse_unary();
vcc_expr_t *vcc_expr_parse_primary();

void vcc_expr_print(vcc_expr_t *root, int depth);

/* ========= STATEMENTS ========== */
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

vcc_stmt_t *vcc_stmt_new();
void vcc_stmt_free(vcc_stmt_t *stmt);

/* ========= PARSER ========== */
typedef struct _vcc_parser_err_t {
  int code;
  buf_t *msg;
} vcc_parser_err_t;

typedef struct _vcc_parser_t {
  vtoken_t *previous;
  vtoken_t *current;
  vtoken_t *next;

  int stacks; // parenthesis stacks
  vcc_parser_err_t err;
} vcc_parser_t;

typedef struct _vcc_node_t {
  int type;
  union {
    vcc_stmt_t *stmt;
    vcc_expr_t *expr;
    vcc_func_t *func;
  } value;
  struct _vcc_node_t *next;
} vcc_node_t;

void vcc_parser_init();
void vcc_parser_finish();
int vcc_parser_continuable();

vcc_node_t *vcc_node_new();
void vcc_node_free(vcc_node_t *node);

void vcc_parser_advance();

vcc_node_t *vcc_parse();

#endif