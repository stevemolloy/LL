#include <stdio.h>
#include <strings.h>

#define SDM_LIB_IMPLEMENTATION
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

char *token_strings[TOKEN_TYPE_COUNT] = {
  [TOKEN_TYPE_SYMBOL] =     "TOKEN_TYPE_SYMBOL",
  [TOKEN_TYPE_NUMBER] =     "TOKEN_TYPE_NUMBER",
  [TOKEN_TYPE_ASSIGNMENT] = "TOKEN_TYPE_ASSIGNMENT",
  [TOKEN_TYPE_ADD] =        "TOKEN_TYPE_ADD",
  [TOKEN_TYPE_MULT] =       "TOKEN_TYPE_MULT",
  [TOKEN_TYPE_SUB] =        "TOKEN_TYPE_SUB",
  [TOKEN_TYPE_DIV] =        "TOKEN_TYPE_DIV",
  [TOKEN_TYPE_OPAREN] =     "TOKEN_TYPE_OPAREN",
  [TOKEN_TYPE_CPAREN] =     "TOKEN_TYPE_CPAREN",
  [TOKEN_TYPE_SEMICOLON] =  "TOKEN_TYPE_SEMICOLON",
  [TOKEN_TYPE_COLON] =      "TOKEN_TYPE_COLON",
  [TOKEN_TYPE_COMMA] =      "TOKEN_TYPE_COMMA",
  [TOKEN_TYPE_COMMENT] =    "TOKEN_TYPE_COMMENT",
  [TOKEN_TYPE_STRING] =     "TOKEN_TYPE_STRING",
};

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

int main(void) {
  const char *input_filename = "small_example.ll";
  char *buffer = sdm_read_entire_file(input_filename);

  sdm_string_view source = sdm_cstr_as_sv(buffer);

  printf(SDM_SV_F"\n", SDM_SV_Vals(source));

  TokenArray token_array = {0};
  SDM_ENSURE_ARRAY_MIN_CAP(token_array, 1024);
  if (!tokenise_input_file(&source, &token_array)) return 1;

  for (size_t i=0; i<token_array.length; i++) {
    Token token = token_array.data[i];
    printf("(%s)  ==>  "SDM_SV_F" (line %zu, column %zu)\n", token_strings[token.type], SDM_SV_Vals(token.content), token.loc.line+1, token.loc.col+1);
  }

  return 0;
}

size_t starts_with_float(const char *input) {
  char *endptr;

  errno = 0;
  strtod(input, &endptr);
  if (errno != 0) return 0;
  size_t retval = endptr - input;
  return retval;
}

bool isvalididchar(char c) {
  return isalnum(c) | (c == '_');
}

bool string_starts_with(const char *input, const char *needle) {
  return strncasecmp(input, needle, strlen(needle))==0;
}

void sv_trim_follow(sdm_string_view *SV, Loc *loc) {
  while (SV->length > 0 && isspace(SV->data[0])) {
    if (SV->data[0] == '\n') {
      loc->line++;
      loc->col = 0;
    } else {
      loc->col++;
    }
    SV->data++;
    SV->length--;
  }
}

sdm_string_view sv_chop_follow(sdm_string_view *SV, size_t len, Loc *loc) {
  sdm_string_view retval = {
    .length = len<SV->length ? len : SV->length,
    .data = SV->data
  };
  for (size_t i=0; SV->length>0 && i<len; i++) {
    if (SV->data[0] == '\n') {
      loc->line++;
      loc->col = 0;
    } else {
      loc->col++;
    }
    SV->data++;
    SV->length--;
  }
  return retval;
}

bool tokenise_input_file(sdm_string_view *file_contents, TokenArray *token_array) {
  Loc loc = {0};
  sv_trim_follow(file_contents, &loc);

  while (file_contents->length > 0) {
    Token token = {0};
    token.loc = loc;

    size_t len = starts_with_float(file_contents->data);
    size_t jump_len = 1;
    if (isalpha(file_contents->data[0])) {
      token.type = TOKEN_TYPE_SYMBOL;
      jump_len = 0;
      while (isvalididchar(file_contents->data[jump_len])) jump_len++;
    } else if (len > 0) {
      token.type = TOKEN_TYPE_NUMBER;
      jump_len = len;
    } else if (string_starts_with(file_contents->data, "//")) {
      token.type = TOKEN_TYPE_COMMENT;
      jump_len = 0;
      while (file_contents->data[jump_len] != '\n') jump_len++;
    } else if (file_contents->data[0] == '"') {
      token.type = TOKEN_TYPE_STRING;
      while (file_contents->data[jump_len] != '"') jump_len++;
    }
    else if (file_contents->data[0] == '=') token.type = TOKEN_TYPE_ASSIGNMENT;
    else if (file_contents->data[0] == ';') token.type = TOKEN_TYPE_SEMICOLON;
    else if (file_contents->data[0] == '/') token.type = TOKEN_TYPE_DIV;
    else if (file_contents->data[0] == ':') token.type = TOKEN_TYPE_COLON;
    else if (file_contents->data[0] == ',') token.type = TOKEN_TYPE_COMMA;
    else if (file_contents->data[0] == '*') token.type = TOKEN_TYPE_MULT;
    else if (file_contents->data[0] == '-') token.type = TOKEN_TYPE_SUB;
    else if (file_contents->data[0] == '(') token.type = TOKEN_TYPE_OPAREN;
    else if (file_contents->data[0] == ')') token.type = TOKEN_TYPE_CPAREN;
    else {
      fprintf(stderr, "Trying to parse an unknown character, %c\n", file_contents->data[0]);
      return false;
    }

    token.content = sv_chop_follow(file_contents, jump_len, &loc);

    SDM_ARRAY_PUSH((*token_array), token);

    sv_trim_follow(file_contents, &loc);
  }
  
  return true;
}

