#include "src/lexer.h"
#include "src/utils.h"

int print_tok(vtoken_t *t) {
    printf("token.type = %d\n", t->type);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("%s [file name]\n", argv[0]);
        return -1;
    }
    vtoken_t *t;
    lex_init(argv[1]);
    do {
        t = lex();
        print_tok(t);
    } while (t->type != TOKEN_EOF);
}
