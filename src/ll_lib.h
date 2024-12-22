#ifndef _LL_LIB_H
#define _LL_LIB_H

#include <stdlib.h>

#include "sdm_lib.h"

typedef enum {
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_NUMBER,
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
  TOKEN_TYPE_COMMENT,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_COUNT,
} TokenType;

typedef struct {
  size_t line;
  size_t col;
} Loc;

typedef struct {
  TokenType type;
  sdm_string_view content;
  Loc loc;
} Token;

typedef struct {
  size_t capacity;
  size_t length;
  Token *data;
} TokenArray;

bool tokenise_input_file(sdm_string_view *file_contents, TokenArray *token_array);
size_t starts_with_float(const char *input);
bool isvalididchar(char c);
bool string_starts_with(const char *input, const char *needle);
void sv_trim_follow(sdm_string_view *SV, Loc *loc);
sdm_string_view sv_chop_follow(sdm_string_view *SV, size_t len, Loc *loc);
bool tokenise_input_file(sdm_string_view *file_contents, TokenArray *token_array);

#endif //_LL_LIB_H

