#include "lexer.h"
#include "mem.h"
#include "parser.h"
#include <stdio.h>

#define PHASE "generating"
#define BUFFER_MAX_SIZE 8096

buf_t *buffer;

void vcc_generator_init() { buffer = buf_new(BUFFER_MAX_SIZE); }

static void nasm_expr(vcc_expr_t *expr, int depth) {
  if (!expr)
    return;

  vcc_expr_t *lhs = expr->lhs;
  vcc_expr_t *rhs = expr->rhs;
  if (expr->arity == 2 && lhs->arity == 0 && rhs->arity == 0) {
    sprintf(buffer->s, "mov r8, %d\nmov r9, %d\n", lhs->atomic.number,
            rhs->atomic.number);

    switch (expr->opr) {
    case TOKEN_ADD:
      sprintf(buffer->s, "add r8, r9\n");
      break;
    case TOKEN_SUB:
      sprintf(buffer->s, "sub r8, r9\n");
      break;
    case TOKEN_ASTERISK:
      sprintf(buffer->s, "mul r8, r9\n");
      break;
    case TOKEN_DIV:
      sprintf(buffer->s, "div r8, r9\n");
      break;
    }

    sprintf(buffer->s, "push r8\n");
    return;
  }

  nasm_expr(lhs, depth + 1);
  nasm_expr(rhs, depth + 1);
}

static char *vcc_generate_stmt_return(vcc_stmt_t *stmt) { return buffer->s; }

static char *vcc_generate_stmt(vcc_stmt_t *stmt) {
  switch (stmt->type) {
  case STMT_TYPE_IF:
    fatals("not implemented for this type yet\n");
    break;
  case STMT_TYPE_RETURN:
    return vcc_generate_stmt_return(stmt);
  }
  return NULL;
}

char *vcc_generate(vcc_node_t *node) {
  switch (node->type) {
  case VCC_NODE_FUNC:
    fatals("not implemented for this type yet\n");
    break;
  case VCC_NODE_STMT:
    return vcc_generate_stmt(node->value.stmt);
  }
}