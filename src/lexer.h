#ifndef _LEXER_H_
#define _LEXER_H_

#include "error.h"
#include "mem.h"
#include "vcc.h"
#include <ctype.h>

#define BUF_EOF '\0'
#define BUF_MAX_SIZE 1024     // token content in buffer
#define NESTING_MAX_LEVEL 128 // nesting comments

enum { LEX_ERR_NONE, LEX_ERR_IO, LEX_ERR_NOMEM, LEX_ERR_UNKNOWN };

/* token types
 */
enum {
  /* special tokens
   */
  TOKEN_EOF, // end of file
  TOKEN_IDENTIFIER,
  TOKEN_INT,
  TOKEN_FLOAT,
  TOKEN_CHAR,
  TOKEN_STR,
  /* punctuations
   */
  TOKEN_COLON,     // :
  TOKEN_LPAREN,    // (
  TOKEN_RPAREN,    // )
  TOKEN_LBRACE,    // {
  TOKEN_RBRACE,    // }
  TOKEN_LBRACKET,  // [
  TOKEN_RBRACKET,  // ]
  TOKEN_COMMA,     // ,
  TOKEN_DOT,       // .
  TOKEN_QUESTION,  // ?
  TOKEN_SEMICOLON, // ;
  /* bitwise operators
   */
  TOKEN_XOR,    // ^
  TOKEN_OR,     // |
  TOKEN_AND,    // &
  TOKEN_LSHIFT, // <<
  TOKEN_RSHIFT, // >>
  /* math operators
   */
  TOKEN_ADD,      // +
  TOKEN_SUB,      // -
  TOKEN_ASTERISK, // *
  TOKEN_DIV,      // /
  TOKEN_MOD,      // %
  TOKEN_EQ,       // ==
  TOKEN_NOT_EQ,   // !=
  TOKEN_LT,       // <
  TOKEN_GT,       // >
  TOKEN_LTEQ,     // <=
  TOKEN_GTEQ,     // >=
  TOKEN_INC,      // ++
  TOKEN_DEC,      // --
  /* logic operators
   */
  TOKEN_TERNARY, // !
  TOKEN_AND_AND, // &&
  TOKEN_OR_OR,   // ||
  /* assign operators
   */
  TOKEN_ASSIGN,        // =
  TOKEN_ADD_ASSIGN,    // +=
  TOKEN_SUB_ASSIGN,    // -=
  TOKEN_OR_ASSIGN,     // |=
  TOKEN_AND_ASSIGN,    // &=
  TOKEN_XOR_ASSIGN,    // ^=
  TOKEN_LSHIFT_ASSIGN, // <<=
  TOKEN_RSHIFT_ASSIGN, // >>=
  TOKEN_MUL_ASSIGN,    // *=
  TOKEN_DIV_ASSIGN,    // /=
  TOKEN_MOD_ASSIGN,    // %=

  TOKEN_POINTER, // ->

  /* keyword tokens
   */
  TOKEN_KWORD_BREAK,
  TOKEN_KWORD_CASE,
  TOKEN_KWORD_CONST,
  TOKEN_KWORD_CONTINUE,
  TOKEN_KWORD_DEFAULT,
  TOKEN_KWORD_DO,
  TOKEN_KWORD_ELSE,
  TOKEN_KWORD_ENUM,
  TOKEN_KWORD_FOR,
  TOKEN_KWORD_GOTO,
  TOKEN_KWORD_IF,
  TOKEN_KWORD_RETURN,
  TOKEN_KWORD_SIZEOF,
  TOKEN_KWORD_STATIC,
  TOKEN_KWORD_STRUCT,
  TOKEN_KWORD_SWITCH,
  TOKEN_KWORD_TYPEDEF,
  TOKEN_KWORD_UNION,
  TOKEN_KWORD_WHILE,
  /* others
   */
  NUMBER_OF_TOKENS
};

extern const char *token_names[];

typedef struct _vtoken_t {
  int type;
  union {
    int i;
    float f;
    buf_t *s;
  } value;

  int line;
  int col;
} vtoken_t;

// vtoken_t *vtoken_new(int, int, int);
// vtoken_t *vtoken_new_from_buf(int);
void vtoken_free(vtoken_t *);

typedef struct _lex_error_t {
  int code;
  int line;
  buf_t *s;
} lex_error_t;

typedef struct _vcc_lexer_t {
  buf_t *filebuf;         // code buffer
  int ptr;                // pointer to file buffer
  char fname[256];        // name of lexed file
  int line;               // current line
  int col;                // current column
  int c;                  // current char
  char buf[BUF_MAX_SIZE]; // buffer to save temp stream
  int buflen;             // len of buf for scanning
  int lastlex;            // return value of the last lex call
} vcc_lexer_t;

int vcc_lexer_init(const char *fname);
int vcc_lexer_finish();
vtoken_t *vcc_lex();

#endif
