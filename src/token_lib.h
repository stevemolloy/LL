#ifndef _LL_LIB_H
#define _LL_LIB_H

#include "sdm_lib.h"
#include <stdint.h>
#include <stdlib.h>

#define SDM_ARRAY_LENGTH(array) sizeof((array)) / sizeof((array[0]))

typedef struct {
  const char *filename;
  sdm_string_view contents;
  size_t line;
  size_t col;
  size_t index;
} Parser;

typedef enum {
  TOKEN_TYPE_UNKNOWN = 0,
  TOKEN_TYPE_ID,
  TOKEN_TYPE_FLOAT,
  TOKEN_TYPE_INT,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_ASSIGNMENT,
  TOKEN_TYPE_ADD,
  TOKEN_TYPE_MULT,
  TOKEN_TYPE_SUB,
  TOKEN_TYPE_DIV,
  TOKEN_TYPE_OPAREN,
  TOKEN_TYPE_CPAREN,
  TOKEN_TYPE_SEMICOLON,
  TOKEN_TYPE_COLON,
  TOKEN_TYPE_COMMA,
  TOKEN_TYPE_POINT,
  TOKEN_TYPE_EOF,
  TOKEN_TYPE_QUOTEMARK,
  TOKEN_TYPE_COUNT,
} TokenType;

extern char *TT_string[];

typedef enum {
  KEYWORD_LET = 0,
  KEYWORD_COUNT,
} KeyWords;

typedef struct { char *value; } IDToken;
typedef struct { double value; } FloatToken;
typedef struct { int64_t value; } IntToken;
typedef struct { char *value; } StringToken;
typedef struct { KeyWords value; } KeyWordToken;

typedef struct {
  TokenType token_type;
  union {
    IDToken id_token;
    FloatToken float_token;
    IntToken int_token;
    StringToken str_token;
    KeyWordToken kw_token;
  } as;
  Parser source;
} Token;

typedef struct {
  size_t capacity;
  size_t length;
  Token *data;
} TokenArray;

void find_and_apply_keywords(TokenArray *t_array);
bool validation_token_array(const TokenArray *t_array);

bool starts_with_comment(Parser parser);
size_t starts_with_float(Parser parser);
Token get_next_token(Parser *parser);
void tokenise_input_file(Parser *parser, TokenArray *token_array);
void parser_trim(Parser *parser);
void parser_chop(Parser *parser, size_t len);
void print_token_array(TokenArray token_array);

#endif // !_LL_LIB_H

