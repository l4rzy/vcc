#include "parser.h"
#include "lexer.h"
#include "mem.h"
#include <stdarg.h>
#include <strings.h>

#define PHASE "parsing"

static vcc_parser_t P;

#define CURRENT P.current
#define NEXT P.next
#define PREVIOUS P.previous

void vcc_parser_init() {
  bzero(&P, sizeof(vcc_parser_t));
  P.err.code = VCC_PARSER_ERR_NONE;
}

static void advance() {
  if (!CURRENT) {
    CURRENT = vcc_lex();
    NEXT = vcc_lex();
  } else {
    vtoken_free(PREVIOUS);
    // preload a token
    PREVIOUS = CURRENT;
    CURRENT = NEXT;
    NEXT = vcc_lex();
  }

  if (!PREVIOUS) {
    logf("current at: [%s] %s\n", token_names[CURRENT->type],
         token_names[NEXT->type]);
    return;
  }
  logf("current at: %s [%s] %s\n", token_names[PREVIOUS->type],
       token_names[CURRENT->type], token_names[NEXT->type]);
}

void vcc_parser_advance() { advance(); }

// expects the CURRENT token to be something, and continues
static void consume(int type) {
  logf("consuming %s\n", token_names[type]);
  advance();
  if (NEXT->type == type) {
    advance();
    return;
  }
  printf("expect %s, received %s\n", token_names[type],
         token_names[NEXT->type]);
}

static int reached_eof() {
  if (CURRENT) {
    return CURRENT->type == TOKEN_EOF;
  }
  return 0;
}

int vcc_parser_continuable() {
  return !reached_eof() && P.err.code == VCC_PARSER_ERR_NONE;
}

vcc_node_t *vcc_node_new() {
  vcc_node_t *node = malloc(sizeof(vcc_node_t));
  bzero(node, sizeof(vcc_node_t));
  return node;
}

/* ================ EXPRESSIONS ================= */
static int match(int num, ...) {
  va_list args;
  va_start(args, num);

  for (int i = 0; i < num; ++i) {
    if (CURRENT->type == va_arg(args, int)) {
      va_end(args);
      return 1;
    }
  }
  return 0;
}

void vcc_expr_print(vcc_expr_t *root, int depth) {
  printf("%*s\t%d\n", depth, token_names[root->opr], root->atomic.number);
  if (root->lhs) {
    vcc_expr_print(root->lhs, depth + 8);
  }
  if (root->rhs) {
    vcc_expr_print(root->rhs, depth + 8);
  }
}

vcc_expr_t *vcc_expr_parse() {
  logs("parsing an expression\n");
  return vcc_expr_parse_equality();
}

vcc_expr_t *vcc_expr_parse_equality() {
  vcc_expr_t *lhs = vcc_expr_parse_comparison();

  while (match(2, TOKEN_NOT_EQ, TOKEN_EQ)) {
    int opr = CURRENT->type;
    logf("catched opr: %s\n", token_names[opr]);
    advance();
    vcc_expr_t *rhs = vcc_expr_parse_comparison();

    lhs = vcc_expr_new_binary(lhs, opr, rhs);
  }

  return lhs;
}

vcc_expr_t *vcc_expr_parse_comparison() {
  vcc_expr_t *lhs = vcc_expr_parse_term();

  while (match(4, TOKEN_LT, TOKEN_GT, TOKEN_LTEQ, TOKEN_GTEQ)) {
    int opr = CURRENT->type;
    logf("catched opr: %s\n", token_names[opr]);
    advance();
    vcc_expr_t *rhs = vcc_expr_parse_term();

    lhs = vcc_expr_new_binary(lhs, opr, rhs);
  }

  return lhs;
}

vcc_expr_t *vcc_expr_parse_term() {
  vcc_expr_t *lhs = vcc_expr_parse_factor();

  while (match(2, TOKEN_ADD, TOKEN_SUB)) {
    int opr = CURRENT->type;
    logf("catched opr: %s\n", token_names[opr]);
    advance();
    vcc_expr_t *rhs = vcc_expr_parse_factor();

    lhs = vcc_expr_new_binary(lhs, opr, rhs);
  }

  return lhs;
}

vcc_expr_t *vcc_expr_parse_factor() {
  vcc_expr_t *lhs = vcc_expr_parse_unary();

  while (match(2, TOKEN_DIV, TOKEN_ASTERISK)) {
    int opr = CURRENT->type;
    logf("catched opr: %s\n", token_names[opr]);
    advance();
    vcc_expr_t *rhs = vcc_expr_parse_unary();

    lhs = vcc_expr_new_binary(lhs, opr, rhs);
  }

  return lhs;
}

vcc_expr_t *vcc_expr_parse_unary() {
  if (match(1, TOKEN_NOT)) {
    int opr = CURRENT->type;
    logf("catched opr: %s\n", token_names[opr]);
    advance();
    vcc_expr_t *rhs = vcc_expr_parse_unary();

    return vcc_expr_new_unary(opr, rhs);
  }

  return vcc_expr_parse_primary();
}

vcc_expr_t *vcc_expr_parse_primary() {
  if (match(1, TOKEN_INT)) {
    logs("primary is an integer\n");
    advance();
    return vcc_expr_new_atomic_int(PREVIOUS->value.i);
  }

  if (match(1, TOKEN_KWORD_NULL)) {
    logs("primary is a null\n");
    advance();
    return vcc_expr_new_atomic_null();
  }

  if (match(1, TOKEN_LPAREN)) {
    logs("primary is a grouping\n");
    advance();
    vcc_expr_t *expr = vcc_expr_parse();

    logf("ended at %s\n", token_names[CURRENT->type]);
    // TODO: expect the RPAREN here
    return expr;
  }
  return NULL;
}

vcc_expr_t *vcc_expr_new() {
  vcc_expr_t *expr = xalloc(sizeof(vcc_expr_t));
  return expr;
}

vcc_expr_t *vcc_expr_new_binary(vcc_expr_t *lhs, int opr, vcc_expr_t *rhs) {
  vcc_expr_t *expr = vcc_expr_new();
  expr->arity = 2;
  expr->opr = opr;
  expr->lhs = lhs;
  expr->rhs = rhs;

  return expr;
}

vcc_expr_t *vcc_expr_new_unary(int opr, vcc_expr_t *rhs) {
  vcc_expr_t *expr = vcc_expr_new();
  expr->arity = 1;
  expr->opr = opr;
  expr->rhs = rhs;

  return expr;
}

vcc_expr_t *vcc_expr_new_atomic_int(int val) {
  vcc_expr_t *expr = vcc_expr_new();
  expr->arity = 0;
  expr->opr = TOKEN_INT;
  expr->atomic.number = val;

  return expr;
}

vcc_expr_t *vcc_expr_new_atomic_null() { return vcc_expr_new_atomic_int(0); }

void vcc_expr_free(vcc_expr_t *expr) {
  if (expr) {
    vcc_expr_free(expr->lhs);
    vcc_expr_free(expr->rhs);
    vcc_expr_free(expr->next);
    free(expr);
  }
}

/* ======== STATEMENTS ======== */
vcc_stmt_t *vcc_stmt_new() {
  vcc_stmt_t *stmt = xalloc(sizeof(vcc_stmt_t));
  return stmt;
}

void vcc_stmt_free(vcc_stmt_t *stmt) {
  if (stmt) {
    vcc_expr_free(stmt->condition);
    vcc_expr_free(stmt->expr);
  }
}

#define expect(expectation, error)                                             \
  do {                                                                         \
    if (CURRENT->type != expectation) {                                        \
      logs("did not meet expectation: ");                                      \
      printf("expect %s, received %s\n", token_names[expectation],             \
             token_names[CURRENT->type]);                                      \
      P.err.code = error;                                                      \
    } else {                                                                   \
      logf("met expectation: %s\n", token_names[CURRENT->type]);               \
    }                                                                          \
  } while (0)

vcc_node_t *vcc_parser_stmt_if() {
  logs("parsing an if statement\n");
  advance();
  vcc_stmt_t *stmt = vcc_stmt_new();
  expect(TOKEN_LPAREN, VCC_PARSER_ERR_WRONG_STMT);
  advance();
  stmt->condition = vcc_expr_parse();
  vcc_expr_print(stmt->condition, 2);
  expect(TOKEN_RPAREN, VCC_PARSER_ERR_WRONG_STMT);
  advance();
  expect(TOKEN_LBRACE, VCC_PARSER_ERR_WRONG_STMT);
  vcc_node_t *node = vcc_node_new();

  node->type = VCC_NODE_STMT_IF;
  node->value.stmt = stmt;
  return node;
}
#undef expect

vcc_node_t *vcc_parse() {
  advance();
  switch (CURRENT->type) {
  case TOKEN_KWORD_IF:
    return vcc_parser_stmt_if();

  case TOKEN_KWORD_WHILE:
    break;

  case TOKEN_KWORD_RETURN:
    break;
  }
  // TODO:
  return NULL;
}
