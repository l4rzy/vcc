#include "src/lexer.h"
#include "src/mem.h"
#include "src/parser.h"

int print_token(vtoken_t *t) {
  printf("(%s", token_names[t->type]);
  switch (t->type) {
  case TOKEN_INT:
    printf(": %d)\n", t->value.i);
    break;
  case TOKEN_FLOAT:
    printf(": %f)\n", t->value.f);
    break;
  default:
    if (t->value.s)
      printf(": \"%s\")\n", t->value.s->s);
    break;
  }
  return 0;
}

void test_lexer(char *fname) {
  vtoken_t *t;
  vcc_lexer_init(fname);
  do {
    t = vcc_lex();
    if (t == NULL) {
      return;
    }
    print_token(t);
    vtoken_free(t);
  } while (t->type != TOKEN_EOF);
  vcc_lexer_finish();
}

void node_inspect(vcc_node_t *node) {
  if (!node)
    return;

  printf("parsed node %s\n", node_types[node->type]);
  if (node->type == VCC_NODE_STMT) {
    printf("stmt type: %s\n", stmt_types[node->value.stmt->type]);
    vcc_expr_print(node->value.stmt->expr, 2);
    vcc_expr_print(node->value.stmt->condition, 2);
  }
}

void test_parser(char *fname) {
  vcc_lexer_init(fname);
  vcc_parser_init();

  while (vcc_parser_continuable()) {
    vcc_node_t *node = vcc_parse();
    node_inspect(node);
    vcc_node_free(node);
  }
  vcc_parser_finish();
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("%s l [filename]\n%s p [filename]\n", argv[0], argv[0]);
    return -1;
  }
  if (!strcmp(argv[1], "l") || !strcmp(argv[1], "lex")) {
    test_lexer(argv[2]);
    return 0;
  }
  if (!strcmp(argv[1], "p") || !strcmp(argv[1], "parse")) {
    test_parser(argv[2]);
    return 0;
  }
  return 0;
}
