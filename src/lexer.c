#include "lexer.h"

/* for error emitting
 */
#define PHASE "lexing"

/* internal variables
 */

static buf_t *filebuf;  // code buffer
static int filebufptr;  // pointer to file buffer
static char fname[256]; // name of lexing file

static int line;               // current line
static int col;                // current column
static int c;                  // current char
static char buf[BUF_MAX_SIZE]; // buffer to save temp stream
static int buflen;             // len of buf for scanning

static int lastlex; // return value of the last lex call

// TODO: save internal variables to an object
// static vcc_lexer_t *lexer;

/* static funtion declarations
 */
static void reset_buf();
static char peek(int);
static int discard_until(char);
static char next(int);

#define KWORD(kw) [KWORD_##kw] = #kw,

/* table of reserved keywords
 */
static const char *keyword_tbl[] = {
    KWORD(enum)
    KWORD(struct)
    KWORD(union)
    KWORD(const)
    KWORD(static)
    KWORD(goto)
    KWORD(sizeof)
    KWORD(break)
    KWORD(continue)
    KWORD(return)
    KWORD(if)
    KWORD(else)
    KWORD(while)
    KWORD(do)
    KWORD(for)
    KWORD(switch)
    KWORD(case)
    KWORD(typedef)
    KWORD(default)
};

#undef KWORD

#define TOKEN(tok) [TOKEN_##tok] = #tok,
const char *token_names[] = {
    TOKEN(EOF)        // end of file
    TOKEN(KEYWORD)    // a pre-defined keyword
    TOKEN(IDENTIFIER) //
    TOKEN(INT)        //
    TOKEN(FLOAT)      //
    TOKEN(CHAR)       //
    TOKEN(STR)        //
    /* punctuations
     */
    TOKEN(COLON)     // :
    TOKEN(LPAREN)    // (
    TOKEN(RPAREN)    // )
    TOKEN(LBRACE)    // {
    TOKEN(RBRACE)    // }
    TOKEN(LBRACKET)  // [
    TOKEN(RBRACKET)  // ]
    TOKEN(COMMA)     // )
    TOKEN(DOT)       // .
    TOKEN(QUESTION)  // ?
    TOKEN(SEMICOLON) // ;
    /* bitwise operators
     */
    TOKEN(XOR)    // ^
    TOKEN(OR)     // |
    TOKEN(AND)    // &
    TOKEN(LSHIFT) // <<
    TOKEN(RSHIFT) // >>
    /* math operators
     */
    TOKEN(ADD)      // +
    TOKEN(SUB)      // -
    TOKEN(ASTERISK) // *
    TOKEN(DIV)      // /
    TOKEN(MOD)      // %
    TOKEN(EQ)       // ==
    TOKEN(NOT_EQ)   // !=
    TOKEN(LT)       // <
    TOKEN(GT)       // >
    TOKEN(LTEQ)     // <=
    TOKEN(GTEQ)     // >=
    TOKEN(INC)      // ++
    TOKEN(DEC)      // --
    /* logic operators
     */
    TOKEN(TERNARY) // !
    TOKEN(AND_AND) // &&
    TOKEN(OR_OR)   // ||
    /* assign operators
     */
    TOKEN(ASSIGN)        // =
    TOKEN(ADD_ASSIGN)    // +=
    TOKEN(SUB_ASSIGN)    // -=
    TOKEN(OR_ASSIGN)     // |=
    TOKEN(AND_ASSIGN)    // &=
    TOKEN(XOR_ASSIGN)    // ^=
    TOKEN(LSHIFT_ASSIGN) // <<=
    TOKEN(RSHIFT_ASSIGN) // >>=
    TOKEN(MUL_ASSIGN)    // *=
    TOKEN(DIV_ASSIGN)    // /=
    TOKEN(MOD_ASSIGN)    // %=
    TOKEN(POINTER)       // ->
};
#undef TOKEN

/* resets temp buffer
 */
static void reset_buf() { bzero(buf, BUF_MAX_SIZE); }

/* creates new token from filebuf
 */
static vtoken_t *vtoken_new(int type, int start, int len) {
  vtoken_t *tok = xalloc(sizeof(vtoken_t));
  if (start < 0) {
    tok->type = type;
    tok->value = NULL;
    return tok;
  }
  tok->type = type;
  tok->value = buf_new_from_mem(filebuf->s + start, len);
  return tok;
}

/* creates new token from buf
 */
static vtoken_t *vtoken_new_from_buf(int type) {
  vtoken_t *tok = xalloc(sizeof(vtoken_t));
  tok->type = type;
  tok->value = buf_new_from_mem(buf, buflen);
  return tok;
}

void vtoken_free(vtoken_t *token) { free(token->value); }

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
  assert(filebufptr + n <= filebuf->len);
  filebufptr += n;
  c = filebuf->s[filebufptr];
  ++col;
  if (c == '\n') {
    ++line;
    col = 0;
  }
  return c;
}

/* checks if current char is digits
 */
static int is_digit(char chr) { return (chr >= '0' && chr <= '9'); }

/* checks if current char is alpha
 */
static int is_alpha(char chr) {
  return ((chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z') ||
          (chr == '_'));
}

/* checks if current char is space
 */
static int is_space(char chr) { return (chr == ' '); }

/* checks if current char is legit for the rest of an identifier
 */
static int is_id(char chr) { return (is_digit(chr) || is_alpha(chr)); }

/* =================== SCANNERS ==================== */
static vtoken_t *scan_char() {
  // set token type
  next(1); // skip first delimiter;
  if (c == '\'') {
    errors("Empty between character delimiter\n");
    return NULL;
  }
  if (c == '\\' && peek(2) == '\'') {
    vtoken_t *t = vtoken_new(TOKEN_CHAR, filebufptr, 2);
    next(3);
    return t;
  } else if (peek(1) == '\'') { // a normal char
    vtoken_t *t = vtoken_new(TOKEN_CHAR, filebufptr, 1);
    next(2);
    return t;
  } else {
    lastlex = 1;
    errors("Wrong or unsupported character, or use `\"` if it is a string");
    return NULL;
  }
  return NULL;
}

static vtoken_t *scan_number() {
  logs("Scanning number\n");
  reset_buf();
  buflen = 0;
  int dotcount = 0;
  int tt = TOKEN_INT; // token type
  while (1) {
    if (c == '.') {
      if (dotcount == 0) {
        tt = TOKEN_FLOAT;
        buf[buflen++] = c;
        next(1);
        continue;
      } else {
        errors("wrong float format\n");
        return NULL;
      }
    }
    if (is_digit(c)) {
      buf[buflen++] = c;
      next(1);
      continue;
    } else {
      break;
    }
  }
  // number postfix etc.
  if (tt == TOKEN_FLOAT && buflen < 2) {
    errors("wrong float\n");
    return NULL;
  }
  if (tt == TOKEN_INT && buflen < 1) {
    errors("wrong int\n");
    return NULL;
  }
  logf("parsed number: %s\n", buf);
  vtoken_t *tok = vtoken_new_from_buf(tt);
  return tok;
}

static vtoken_t *scan_string() {
  int len = -1;
  int start = filebufptr + 1;
  while (1) {
    next(1); // skip the first string delimiter
    ++len;
    if (c == BUF_EOF) {
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
    if (c == BUF_EOF) {
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
  logf("C comment ended on line %d\n", line);
  next(1);
  return 0;
}

/* tests buf to check if it's a keyword
 */
static int is_keyword(vtoken_t *tok) { return 1; }

static vtoken_t *scan_identifier() {
  logs("scanning identifier\n");
  // init buffer state
  reset_buf();
  buflen = 0;
  while (1) {
    if (buflen == 0 && is_alpha(c)) {
      buf[buflen++] = c;
      next(1);
    } else if (buflen > 0 && is_id(c)) {
      buf[buflen++] = c;
      next(1);
    } else {
      break;
    }
  }
  vtoken_t *t = vtoken_new_from_buf(TOKEN_IDENTIFIER);
  return t;
}

int lexer_init(const char *_fname) {
  FILE *fp = fopen(_fname, "rb");
  if (!fp) {
    fatalf("Could not open `%s` to read\n", fname);
  }
  logf("Opened `%s` at %p\n", _fname, (void *)fp);
  // copy file data to buffer
  filebuf = buf_new_from_file(fp);
  // init variables
  strncpy(fname, _fname, 255);
  line = 1;
  col = 0;
  filebufptr = 0;
  assert(filebuf->len > 0);
  c = filebuf->s[filebufptr];
  buflen = 0;
  lastlex = 0;
  return 0;
}

/* free resources
 */
int lexer_finish() {
  logs("lexing done, freeing resources\n");
  buf_free(filebuf);
  return 0;
}

static vtoken_t *next_token() {
_lex_loop:
  logs("lexing the next token\n");
  // skip lexing if the last lex failed
  if (lastlex != 0) {
    logs("Last lex() failed, skipping\n");
    return NULL;
  }
  // ignore white spaces
  if (c != BUF_EOF) {
    if (isspace(c)) {
      // it's safe to continue here since we filter new line in next(1)
      next(1);
      goto _lex_loop;
    }
    logf("line %d col %d | c = '%c'\n", line, col, c);
    switch (c) {
    case '#': // currently treat preprocessing statements as comments
      logf("Preprocessor procedure on line %d\n", line);
      discard_until('\n');
      goto _lex_loop;

    case '\'':
      logf("Scanning char on line %d\n", line);
      return scan_char();

    case '"':
      logf("Scanning string on line %d\n", line);
      return scan_string();

    case '/':
      if (peek(1) == '/') {
        logf("C++ comment on line %d\n", line);
        discard_until('\n');
        goto _lex_loop; // skip to the next real token
      }
      if (peek(1) == '*') {
        logf("C comment on line %d\n", line);
        lastlex = scan_comment();
        if (lastlex == 0) {
          goto _lex_loop; // skip to the next real token
        }
        // return error
        return NULL;
      }
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_DIV_ASSIGN, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_DIV, -1, 0);

    case ';':
      next(1);
      return vtoken_new(TOKEN_SEMICOLON, -1, 0);

    case '(':
      next(1);
      return vtoken_new(TOKEN_LBRACKET, -1, 0);

    case ')':
      next(1);
      return vtoken_new(TOKEN_RBRACKET, -1, 0);

    case '{':
      next(1);
      return vtoken_new(TOKEN_LPAREN, -1, 0);

    case '}':
      next(1);
      return vtoken_new(TOKEN_RPAREN, -1, 0);

    case '*':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_MUL_ASSIGN, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_ASTERISK, -1, 0);

    case ',':
      next(1);
      return vtoken_new(TOKEN_COMMA, -1, 0);
    case '.':
      if (is_digit(peek(1))) {
        return scan_number();
      }
      next(1);
      return vtoken_new(TOKEN_DOT, -1, 0);

    case '[':
      next(1);
      return vtoken_new(TOKEN_LBRACE, -1, 0);
    case ']':
      next(1);
      return vtoken_new(TOKEN_RBRACE, -1, 0);
    case '!':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_NOT_EQ, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_TERNARY, -1, 0);

    case '>':
      // to do shift & shift equal
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_GTEQ, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_GT, -1, 0);

    case '<':
      // to do shift & shift equal
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_LTEQ, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_LT, -1, 0);

    case '=':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_EQ, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_ASSIGN, -1, 0);

    case '|':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_OR_ASSIGN, -1, 0);
      }
      if (peek(1) == '|') {
        next(2);
        return vtoken_new(TOKEN_OR_OR, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_OR, -1, 0);

    case '&':
      if (peek(1) == '&') {
        next(2);
        return vtoken_new(TOKEN_AND_AND, -1, 0);
      }
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_AND_ASSIGN, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_AND, -1, 0);

    case '%':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_MOD_ASSIGN, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_ASSIGN, -1, 0);

    case ':':
      next(1);
      return vtoken_new(TOKEN_COLON, -1, 0);

    case '+':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_ADD_ASSIGN, -1, 0);
      }
      if (peek(1) == '+') {
        next(2);
        return vtoken_new(TOKEN_INC, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_ADD, -1, 0);

    case '-':
      if (peek(1) == '=') {
        next(2);
        return vtoken_new(TOKEN_SUB_ASSIGN, -1, 0);
      }
      if (peek(1) == '>') {
        next(2);
        return vtoken_new(TOKEN_POINTER, -1, 0);
      }
      if (peek(1) == '-') {
        next(2);
        return vtoken_new(TOKEN_DEC, -1, 0);
      }
      next(1);
      return vtoken_new(TOKEN_SUB, -1, 0);

    default:
      // identifiers and keywords don't start with digit, only numbers
      if (is_digit(c)) {
        return scan_number();
      }
      if (is_alpha(c)) {
        return scan_identifier();
      }
      errors("Unknown or unimplemented token!");
      return NULL;
    }
  } else {
    /* add the final token: the EOF
     */
    logs("Reached EOF\n");
    return vtoken_new(TOKEN_EOF, -1, 0);
  }
  return NULL;
}

/* this function is called by parser to
 * get the tokens from stream
 */
vtoken_t *lex() { return next_token(); }
