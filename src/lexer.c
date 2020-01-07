#include "lexer.h"

/* for error emitting
 */
#define PHASE "lexing"

/* internal variables
 */
static buf_t *filebuf;
static int filebufptr; // pointer to file buffer
static char fname[256]; // name of lexing file

static int line; // current line
static int c; // current char
static char buf[BUF_MAX_SIZE]; // buffer to save temp stream
static int buflen; // len of buf for scanning
static vtoken_t *token;

static int lastlex; // return value of the last lex call

/* static funtion declarations
 */
static char peek(int);
static int discard_until(char);
static char next(int);

#define INIT_KWORD(kw) \
    [KWORD_##kw] = #kw,

/* table of reserved keywords
 */
static const char *keyword_tbl[] = {
    [KWORD_none] = "",
    INIT_KWORD(enum)
    INIT_KWORD(struct)
    INIT_KWORD(union)
    INIT_KWORD(const)
    INIT_KWORD(static)
    INIT_KWORD(goto)
    INIT_KWORD(sizeof)
    INIT_KWORD(break)
    INIT_KWORD(continue)
    INIT_KWORD(return)
    INIT_KWORD(if)
    INIT_KWORD(else)
    INIT_KWORD(while)
    INIT_KWORD(do)
    INIT_KWORD(for)
    INIT_KWORD(switch)
    INIT_KWORD(case)
    INIT_KWORD(typedef)
    INIT_KWORD(default)
};

#undef INIT_KWORD

/* creates new token from filebuf
 */
vtoken_t *vtoken_new(int type, int start, int len) {
    vtoken_t *tok = xalloc(sizeof(vtoken_t));
    tok->type = type;
    tok->value = buf_new_from_mem(filebuf->s + start, len);
    return tok;
}

/* creates new token from buf
 */
vtoken_t *vtoken_new_from_buf(int type) {
    vtoken_t *tok = xalloc(sizeof(vtoken_t));
    tok->type = type;
    tok->value = buf_new_from_mem(buf, buflen);
    return tok;
}

void vtoken_free(vtoken_t *token) {
    free(token->value);
}

/* looks for next char in buf without increasing filebufptr
 */
static char peek(int n) {
    int ptr = filebufptr + n;
    assert(ptr < filebuf->len);
    char p = filebuf->s[ptr];
    return p;
}

/* discards all char until reaches `ch`
 */
static int discard_until(char ch) {
    while (next(1)) {
        if (c == ch || c == EOF) {
            return 0;
        }
    }
    return -1;
}

/* gets next char
 */
static char next(int n) {
    assert(filebufptr + n < filebuf->len);
    filebufptr += n;
    c = filebuf->s[filebufptr];
    if (c == '\n') {
        ++line;
    }
    return c;
}

/* eats current char, and goes to next char
 */
static char eat() {
    buf[buflen++] = c;
    next(1);
    return c;
}

/* checks if current char is digits
 */
static int is_digit() {
    return (c >= '0' && c <= '9');
}

/* checks if current char is alpha
 */
static int is_alpha() {
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c == '_'));
}

/* =================== SCANNERS ==================== */
static vtoken_t *scan_char() {
    // set token type
    next(1); // skip first delimiter;
    if (c == '\'') {
        errors("Empty between character delimiter\n");
        return NULL;
    }
    if (c == '\\' && peek(2) == '\'') {
        return vtoken_new(TOKEN_CHAR, filebufptr, 2);
    } else if (peek(1) == '\'') { // a normal char
        return vtoken_new(TOKEN_CHAR, filebufptr, 1);
    } else {
        errors("Wrong or unsupported character");
        return NULL;
    }
    return NULL;
}

static vtoken_t *scan_number() {
    buflen = 0;
    int dotcount = 0;
    int tt = TOKEN_INT;
    while (1) {
        if (c == '.') {
            if (dotcount == 0) {
                tt = TOKEN_FLOAT;
                eat();
                continue;
            } else {
                errors("wrong float format\n");
                return NULL;
            }
        }
        if (is_digit(c)) {
            eat();
            continue;
        } else {
            errors("Invalid number?");
            return NULL;
        }
        // number postfix etc.
        logf("parsed number: %s\n", buf);
        vtoken_t *tok = vtoken_new_from_buf(tt);
        return tok;
    }
    return NULL;
}

static vtoken_t *scan_string() {
    int len = -1;
    int start = filebufptr + 1;
    while (1) {
        next(1); // skip the first string delimiter
        ++len;
        if (c == EOF) {
            errors("String has no ending delimiter\n");
            return NULL;
        }
        if (c == '\\' && peek(1) == '"') {
            next(2);
            len += 2;
            continue;
        }
        if (c == '"') {
            next(1); // skip the second string delimiter, done
            return vtoken_new(TOKEN_STR, start, len);
        }
    }
    return NULL;
}

/* scan and deal with nested C-style comments
 */
static int scan_comment() {
    /* maintain a stack that counts the number of comments in total
     */
    int stack = 1;
    while (stack) {
        next(1); // skip current comment open symbol
        if (c == EOF) {
            errors("Comment has no ending\n");
            return -1;
        }
        if (c == '/' && peek(1) == '*') {
            ++stack;
            next(1);
            continue;
        }
        if (c == '*' && peek(1) == '/') {
            --stack;
            next(1);
            continue;
        }
    }
    return 0;
}

static int identifier_test(int keyword) {
    return 0;
}

static vtoken_t *scan_identifier() {
    logs("scanning identifiers\n");
    // init buffer state
    buflen = 0;
    while (1) {
        if (is_alpha(c)) {
            buf[buflen++] = c;
        }
        next(1);
        if (c == '\n' || c == ' ') {
            logf("ended identifier, buf = %s\n", buf);
            break;
        }
    }
    return vtoken_new_from_buf(TOKEN_IDENTIFIER);
}

int lex_init(const char *_fname) {
    FILE *fp = fopen(_fname, "rb");
    if (!fp) {
        fatalf("Could not open `%s` to read\n", fname);
    }
    logf("Opened `%s` at %p\n", _fname, fp);
    // copy file data to buffer
    filebuf = buf_new_from_file(fp);
    // init variables
    strncpy(fname, _fname, 255);
    line = 1;
    filebufptr = -1;
    buflen = 0;
    lastlex = 0;
    return 0;
}

/* free resources
 */
int lex_fin() {
    buf_free(filebuf);
    return 0;
}

static vtoken_t *next_token() {
_lex_loop:
    logs("lexing next token\n");
    // skip lexing if the last lex failed
    if (lastlex != 0) {
        logs("Last lex() failed, skipping\n");
        return NULL;
    }
    // ignore white spaces
    if (next(1) != EOF) {
        if (isspace(c)) {
            // it's safe to continue here since we filter new line in next(1)
            goto _lex_loop;
        }
        logf("c = %c\n", c);
        if (c == '#') {
            logf("Preprocessor procedure on line %d\n", line);
            discard_until('\n');
            goto _lex_loop;
        }
        if (c == '\'') {
            logf("Scanning char on line %d\n", line);
            return scan_char();
        }
        if (c == '"') {
            logf("Scanning string on line %d\n", line);
            return scan_string();
        }
        if (c == '/') {
            if (peek(1) == '/') {
                logf("C++ comment on line %d\n", line);
                discard_until('\n');
                goto _lex_loop; // skip to the next real token
            }
            if (peek(1) == '*') {
                logf("C comment on line %d\n", line);
                lastlex = scan_comment();
                if (lastlex == 0) {
                    logf("C comment ended on line %d\n", line);
                    goto _lex_loop; // skip to the next real token
                }
                // return error
                return NULL;
            }
            if (peek(1) == '=') {
                // this is the case of division operation
                token->type = TOKEN_DIV_ASSIGN;
                next(1);
                return 0;
            }
        }
        // identifiers and keywords don't start with digit, only numbers
        if (isdigit(c)) {
            return scan_number();
        } else {
            // identifiers and keywords
            return scan_identifier();
        }
        errors("Unknown or unimplemented token!");
        return NULL;
    } else {
        /* add the final token: the EOF
         */
        logs("Reached EOF\n");
        return vtoken_new(TOKEN_EOF, filebufptr, 0);
    }
    return NULL;
}

/* this function is called by parser to
 * get the tokens from stream
 */
vtoken_t *lex() {
    if (lastlex != 0) {
        fatals("Last lex() failed, abort!\n");
    }
    return next_token();
}
