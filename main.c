#include "src/lexer.h"
#include "src/mem.h"

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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("%s [file name]\n", argv[0]);
    return -1;
  }
  vtoken_t *t;
  lexer_init(argv[1]);
  do {
    t = lex();
    if (t == NULL) {
      return 0;
    }
    print_token(t);
    vtoken_free(t);
  } while (t->type != TOKEN_EOF);
  lexer_finish();
}
