#include "parser.h"
#include "lexer.h"

#define PHASE "parsing"

static vcc_parser_t P;

#define CURRENT P.current
#define NEXT P.next

void vcc_parser_init() {
  bzero(&P, sizeof(vcc_parser_t));
  P.error = 0;
}

static void advance() {
  P.current = vcc_lex();
  P.next = vcc_lex();
}

static int reached_eof() {
  if (NEXT) {
    return NEXT->type == TOKEN_EOF;
  }
  if (CURRENT) {
    return CURRENT->type == TOKEN_EOF;
  }
  return 0;
}

int vcc_parser_eof() { return reached_eof() || P.error != 0; }

#define expect(expectation)                                                    \
  if (NEXT->type != expectation) {                                             \
    printf("expect %s, received %s\n", token_names[expectation],               \
           token_names[NEXT->type]);                                           \
    P.error = 1;                                                               \
  } else {                                                                     \
    advance();                                                                 \
  }
void vcc_parse() {
  advance();
  switch (CURRENT->type) {
  case TOKEN_KWORD_IF:
    logs("parsing an if statement\n");
    expect(TOKEN_LPAREN);
    advance();

    break;

  case TOKEN_KWORD_WHILE:
    logs("parsing a while statement\n");
    expect(TOKEN_LPAREN);
    advance();
    break;
  }
}

#undef expect