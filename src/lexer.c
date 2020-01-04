#include "lexer.h"

/* for error emitting
 */
#define PHASE "LEX"

/* internal variables
 */
static char *filebuf;  // the current source code file
static int filebufptr; // pointer to file buffer
static char fname[256]; // name of lexing file

static int line; // current line
static int c; // current char
static char buf[BUF_MAX_SIZE]; // buffer to save temp stream
static int buflen; // len of buf for scanning
static vtoken_t token;

static int lastlex; // return value of the last lex call

/* static funtion declarations
 */
static char peek();
static int discard_until(char);
static char next();

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
    INIT_KWORD(default)
};

#undef INIT_KWORD

/* looks for next char in buf without increasing filebufptr
 */
static char peek(int n) {
    char p = filebuf[filebufptr+n];
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
    c = filebuf[filebufptr += n];
    if (c == '\n') {
        ++line;
    }
    return c;
}

/* eats current char
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
static int scan_char() {
    // set token type
    token.type = TOKEN_CHAR;
    token.ptr_start = filebufptr;
    next(1); // skip first delimiter;
    if (c == '\'') {
        errors("Empty between character delimiter\n");
        return -1;
    }
    if (c == '\\' && peek(1) == '\'') {
        next(1);
        token.ptr_end = filebufptr;
        // save escaped char to token
    }
    if (peek(1) == '\'') {
        next(1); // skip second delimiter
        return 0;
    }
    return 0;
}

static int scan_number() {
    token.type = TOKEN_INT;
    buflen = 0;
    while (1) {
        if (c == '.') {
            token.type = TOKEN_FLOAT;
            eat();
            continue;
        }
        if (isdigit(c)) {
            eat();
            continue;
        } else {
            errors("Invalid number?");
            return -1;
        }
        // number postfix etc.
        logf("parsed number: %s\n", buf);
    }
    return 0;
}

static int scan_string() {
    while (1) {
        next(1); // skip the first string delimiter
        if (c == EOF) {
            errors("String has no ending delimiter\n");
            return -1;
        }
        if (c == '\\' && peek(1) == '"') {
            next(1);
            continue;
        }
        if (c == '"') {
            next(1); // skip the second string delimiter, done
            buf[buflen] = '\0';
            break;
        }
        buf[buflen++] = c;
    }
    buflen = 0;
    return 0;
}

/* scan and deal with nested C-style comments
 */
int scan_comment() {
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

int scan_identifier() {
    
}

long file_size(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long result = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return result;
}

int lex_init(const char *_fname) {
    strncpy(fname, _fname, 255);
    line = 1;
    FILE *fp = fopen(fname, "rb");
    if (!fp) {
        fatalf("Could not open `%s` to read\n", fname);
    }
    logf("Opened `%s` at %p\n", fname, fp);
    buflen = 0;

    // copy file data to buffer
    int len = file_size(fp);
    filebuf = malloc(len+1);
    if (!filebuf) {
        fatals("Can not allocate memory");
    }

    fread(filebuf, len, 1, fp);
    filebufptr = 0;
    
    lastlex = 0;
    return 0;
}

/* free resources
 */
int lex_fin() {
    free(filebuf);
    return 0;
}

static int next_token() {
_lex_loop:
    logs("lexing next token\n");
    // skip lexing if the last lex failed
    if (lastlex != 0) {
        logs("Last lex() failed, skipping\n");
        return -1;
    }
    // ignore white spaces
    if (next(1) != EOF) {
        if (isspace(c)) {
            // it's safe to continue here since we filter new line in next(1)
            goto _lex_loop;
        }
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
                int r = scan_comment();
                if (r == 0) {
                    logf("C comment ended on line %d\n", line);
                    goto _lex_loop; // skip to the next real token
                }
                // return error
                return r;
            }
            if (peek(1) == '=') {
                // this is the case of division operation
                token.type = TOKEN_DIV_ASSIGN;
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
        return -1;
    } else {
        /* add the final token: the EOF
         */
        logs("Reached EOF\n");
        token.type = TOKEN_EOF;
        return 0;
    }
    return 0;
}

/* this function is called by parser to
 * get the tokens from stream
 */
vtoken_t *lex() {
    if (lastlex != 0) {
        fatals("Last lex() failed, abort!\n");
    }

    errorf("line: %d buf pointer: %d\n", line, filebufptr);
    lastlex = next_token();
    if (lastlex == 0) {
        return &token;
    }
    else {
        return NULL;
    }
}
