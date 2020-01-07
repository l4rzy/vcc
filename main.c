#include "src/lexer.h"
#include "src/utils.h"

// print token
int print_tok(vtoken_t *t) {
    printf("token.type = %d\n", t->type);
    if (t->value) {
        printf("token.value = %s\n", t->value->s);
    }
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
        if (t == NULL) {
            return 0;
        }
        print_tok(t);
        vtoken_free(t);
    } while (t->type != TOKEN_EOF);
    lex_fin();
}
