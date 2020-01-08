#ifndef _LEXER_H_
#define _LEXER_H_

#include "common.h"
#include "utils.h"
#include <ctype.h>

#define BUF_EOF                 '\0'
#define BUF_MAX_SIZE            1024        // token content in buffer
#define NESTING_MAX_LEVEL       128         // nesting comments

enum {
    LEX_ERR_NONE,
    LEX_ERR_IO,
    LEX_ERR_NOMEM,
    LEX_ERR_UNKNOWN
};

/* a table and index to save keyworlds
 */
enum {
    KWORD_none,
    KWORD_enum,
    KWORD_struct,
    KWORD_union,
    KWORD_const,
    KWORD_static,
    KWORD_goto,
    KWORD_sizeof,
    KWORD_break,
    KWORD_continue,
    KWORD_return,
    KWORD_if,
    KWORD_else,
    KWORD_while,
    KWORD_do,
    KWORD_for,
    KWORD_switch,
    KWORD_case,
    KWORD_typedef,
    KWORD_default
};

/* token types
 */
enum {
    /* special tokens
     */
    TOKEN_EOF, // end of file
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_STR,
    /* punctuations
     */
    TOKEN_COLON, // :
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }
    TOKEN_LBRACKET, // [
    TOKEN_RBRACKET, // ]
    TOKEN_COMMA, // ,
    TOKEN_DOT, // .
    TOKEN_QUESTION, // ?
    TOKEN_SEMICOLON, // ;
    /* bitwise operators
     */
    TOKEN_XOR, // ^
    TOKEN_OR, // |
    TOKEN_AND, // &
    TOKEN_LSHIFT, // <<
    TOKEN_RSHIFT, // >>
    /* math operators
     */
    TOKEN_ADD, // +
    TOKEN_SUB, // -
    TOKEN_ASTERISK, // *
    TOKEN_DIV, // /
    TOKEN_MOD, // %
    TOKEN_EQ, // ==
    TOKEN_NOT_EQ, // !=
    TOKEN_LT, // <
    TOKEN_GT, // >
    TOKEN_LTEQ, // <=
    TOKEN_GTEQ, // >=
    TOKEN_INC, // ++
    TOKEN_DEC, // --
    /* logic operators
     */
    TOKEN_TERNARY, // !
    TOKEN_AND_AND, // &&
    TOKEN_OR_OR, // ||
    /* assign operators
     */
    TOKEN_ASSIGN, // =
    TOKEN_ADD_ASSIGN, // +=
    TOKEN_SUB_ASSIGN, // -=
    TOKEN_OR_ASSIGN, // |=
    TOKEN_AND_ASSIGN, // &=
    TOKEN_XOR_ASSIGN, // ^=
    TOKEN_LSHIFT_ASSIGN, // <<=
    TOKEN_RSHIFT_ASSIGN, // >>=
    TOKEN_MUL_ASSIGN, // *=
    TOKEN_DIV_ASSIGN, // /=
    TOKEN_MOD_ASSIGN, // %=

    TOKEN_POINTER, // ->
    /* others
     */
    NUM_TOKENS
};

const char* token_names[NUM_TOKENS];

typedef struct _vtoken_t {
    int type;
    buf_t *value;
} vtoken_t;

vtoken_t *vtoken_new(int, int, int);
vtoken_t *vtoken_new_from_buf(int);
void vtoken_free(vtoken_t *);

typedef struct _lex_error_t {
    int code;
    int line;
    buf_t *s;
} lex_error_t;

int lex_init(const char *fname);
int lex_fin();
vtoken_t *lex();

#endif
