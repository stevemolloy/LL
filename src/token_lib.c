#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "token_lib.h"
#include "sdm_lib.h"

char *TT_string[] = {
  [TOKEN_TYPE_UNKNOWN]    = "TOKEN_TYPE_UNKNOWN",
  [TOKEN_TYPE_ID]         = "TOKEN_TYPE_ID",
  [TOKEN_TYPE_FLOAT]      = "TOKEN_TYPE_FLOAT",
  [TOKEN_TYPE_INT]        = "TOKEN_TYPE_INT",
  [TOKEN_TYPE_STRING]     = "TOKEN_TYPE_STRING",
  [TOKEN_TYPE_KEYWORD]    = "TOKEN_TYPE_KEYWORD",
  [TOKEN_TYPE_ASSIGNMENT] = "TOKEN_TYPE_ASSIGNMENT",
  [TOKEN_TYPE_ADD]        = "TOKEN_TYPE_ADD",
  [TOKEN_TYPE_MULT]       = "TOKEN_TYPE_MULT",
  [TOKEN_TYPE_SUB]        = "TOKEN_TYPE_SUB",
  [TOKEN_TYPE_DIV]        = "TOKEN_TYPE_DIV",
  [TOKEN_TYPE_OPAREN]     = "TOKEN_TYPE_OPAREN",
  [TOKEN_TYPE_CPAREN]     = "TOKEN_TYPE_CPAREN",
  [TOKEN_TYPE_SEMICOLON]  = "TOKEN_TYPE_SEMICOLON",
  [TOKEN_TYPE_COLON]      = "TOKEN_TYPE_COLON",
  [TOKEN_TYPE_COMMA]      = "TOKEN_TYPE_COMMA",
  [TOKEN_TYPE_POINT]      = "TOKEN_TYPE_POINT",
  [TOKEN_TYPE_EOF]        = "TOKEN_TYPE_EOF",
  [TOKEN_TYPE_QUOTEMARK]  = "TOKEN_TYPE_QUOTEMARK",
  [TOKEN_TYPE_COUNT]      = "TOKEN_TYPE_COUNT",
};

char *keyword_strings[] = {
  [KEYWORD_LET] = "let",
};
static_assert(
  sizeof(keyword_strings) / sizeof(keyword_strings[0]) == KEYWORD_COUNT, 
  "Wrong number of keywords"
);

void print_token_array(TokenArray token_array) {
  for (size_t i=0; i<token_array.length; i++) {
    Token t = token_array.data[i];
    TokenType tt = t.token_type;
    printf("%s", TT_string[tt]);
    if (tt == TOKEN_TYPE_ID)
      printf(" :: %s", t.as.id_token.value);
    else if (tt == TOKEN_TYPE_INT) 
      printf(" :: %ld", t.as.int_token.value);
    else if (tt == TOKEN_TYPE_FLOAT) 
      printf(" :: %f", t.as.float_token.value);
    else if (tt == TOKEN_TYPE_KEYWORD) 
      printf(" :: %s", keyword_strings[t.as.kw_token.value]);
    printf("\n");
  }
}

char *get_current_tokeniser_string(Tokeniser tokeniser) {
  return tokeniser.contents.data + tokeniser.index;
}

void advance_to_next_line(Tokeniser *tokeniser) {
  while (tokeniser->contents.data[tokeniser->index] != '\n') {
    tokeniser->index++;
  }
  tokeniser->index++;
  if (tokeniser->index < tokeniser->contents.length) {
    tokeniser->line++;
    tokeniser->col = 1;
  }
}

bool starts_with_comment(Tokeniser tokeniser) {
  const char *comment_marker = "//";
  return strncmp(get_current_tokeniser_string(tokeniser), comment_marker, strlen(comment_marker)) == 0;
}

size_t starts_with_float(Tokeniser tokeniser) {
  char *endptr;
  char *input = get_current_tokeniser_string(tokeniser);
  errno = 0;
  strtod(input, &endptr);
  if (errno != 0) return 0;
  size_t retval = endptr - input;
  return retval;
}

char tokeniser_current_char(const Tokeniser *tokeniser) {
  return tokeniser->contents.data[tokeniser->index];
}

bool tokeniser_isalpha(const Tokeniser *tokeniser) {
  return isalpha(tokeniser_current_char(tokeniser));
}

bool tokeniser_is_id_char(const Tokeniser *tokeniser) {
  return 
    tokeniser_isalpha(tokeniser) || 
    tokeniser_current_char(tokeniser) == '_' || 
    isalnum(tokeniser_current_char(tokeniser));
}

Token get_next_token(Tokeniser *tokeniser) {
  tokeniser_trim(tokeniser);

  while (starts_with_comment(*tokeniser)) {
    advance_to_next_line(tokeniser);
    tokeniser_trim(tokeniser);
  };

  Token token = {0};
  memcpy(&token.source, tokeniser, sizeof(*tokeniser));
  size_t len; // Only valid for the float/int part of the code

  if (tokeniser->index >= tokeniser->contents.length) {
    token.token_type = TOKEN_TYPE_EOF;
  } else if ((len = starts_with_float(*tokeniser)) > 0) {
    // Could be an integer or a float
    char *start_ptr = tokeniser->contents.data + tokeniser->index;
    char *end_ptr = start_ptr;
    strtol(start_ptr, &end_ptr, 10);
    if ((end_ptr - start_ptr) == (long)len) {
      token.token_type = TOKEN_TYPE_INT;
      token.as.int_token.value = atoi(start_ptr);
    } else {
      token.token_type = TOKEN_TYPE_FLOAT;
      token.as.float_token.value = atof(start_ptr);
    }
    tokeniser->index += len;
  } else if (tokeniser_isalpha(tokeniser)) {
    size_t start_index = tokeniser->index;
    while (tokeniser_is_id_char(tokeniser)) tokeniser->index++;
    size_t id_len = tokeniser->index - start_index;
    token.token_type = TOKEN_TYPE_ID;
    token.as.id_token.value = SDM_MALLOC((id_len+1) * sizeof(char));
    memcpy(token.as.id_token.value, &tokeniser->contents.data[start_index], id_len);
  } else if (tokeniser_current_char(tokeniser) == ',') {
    token.token_type = TOKEN_TYPE_COMMA;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '.') {
    token.token_type = TOKEN_TYPE_POINT;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '+') {
    token.token_type = TOKEN_TYPE_ADD;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '-') {
    token.token_type = TOKEN_TYPE_SUB;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '=') {
    token.token_type = TOKEN_TYPE_ASSIGNMENT;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '/') {
    token.token_type = TOKEN_TYPE_DIV;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '*') {
    token.token_type = TOKEN_TYPE_MULT;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == ':') {
    token.token_type = TOKEN_TYPE_COLON;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == ';') {
    token.token_type = TOKEN_TYPE_SEMICOLON;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '(') {
    token.token_type = TOKEN_TYPE_OPAREN;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == ')') {
    token.token_type = TOKEN_TYPE_CPAREN;
    tokeniser->index += 1;
  } else if (tokeniser_current_char(tokeniser) == '"') {
    // We have a string.  We have to find the end
    tokeniser->index++;
    size_t str_start = tokeniser->index;
    while ((tokeniser->index < tokeniser->contents.length) && (tokeniser_current_char(tokeniser) != '"')) {
      tokeniser->index++;
    }
    size_t str_len = tokeniser->index - str_start;
    token.token_type = TOKEN_TYPE_STRING;
    token.as.str_token.value = SDM_MALLOC(str_len + 1);
    memset(token.as.str_token.value, 0, str_len + 1);
    memcpy(token.as.str_token.value, tokeniser->contents.data+str_start, str_len);
    tokeniser->index += str_len + 1;
  } else {
    fprintf(stderr, "WARNING: Unsure how to parse '%c'\n", tokeniser->contents.data[tokeniser->index]);
    token.token_type = TOKEN_TYPE_UNKNOWN;
    tokeniser->index += 1;
  }

  return token;
}

void tokenise_input_file(Tokeniser *tokeniser, TokenArray *token_array) {
  sdm_string_view contents = tokeniser->contents;

  while (tokeniser->index < contents.length) {
    SDM_ARRAY_PUSH(*token_array, get_next_token(tokeniser));
  }
}

void tokeniser_trim(Tokeniser *tokeniser) {
  char *text = get_current_tokeniser_string(*tokeniser);
  while (strlen(text) > 0 && isspace(*text)) {
    if (*text == '\n') {
      tokeniser->line++;
      tokeniser->col = 1;
    } else {
      tokeniser->col++;
    }
    tokeniser->index++;
    text = get_current_tokeniser_string(*tokeniser);
  }
}

void tokeniser_chop(Tokeniser *tokeniser, size_t len) {
  sdm_string_view *SV = &tokeniser->contents;
  for (size_t i=0; SV->length>0 && i<len; i++) {
    if (SV->data[0] == '\n') {
      tokeniser->line++;
      tokeniser->col = 0;
    } else {
      tokeniser->col++;
    }
    SV->data++;
    SV->length--;
  }
}

void find_and_apply_keywords(TokenArray *t_array) {
  for (size_t i=0; i<t_array->length; i++) {
    if (t_array->data[i].token_type != TOKEN_TYPE_ID)
      continue;
    Token *t = &t_array->data[i];
    for (size_t j=0; j<KEYWORD_COUNT; j++) {
      if (strcmp(t->as.id_token.value, keyword_strings[j]) == 0) {
        t->token_type = TOKEN_TYPE_KEYWORD;
        t->as.kw_token.value = j;
        break;
      }
    }
  }
}

bool validate_token_array(const TokenArray *t_array) {
  if (t_array->data[t_array->length-1].token_type != TOKEN_TYPE_EOF) {
    fprintf(stderr, "Final token should be TOKEN_TYPE_EOF, but is not. This is a bug in the tokeniser.\n");
    return false;
  }
  int parens_balance = 0;
  for (size_t i=0; i<t_array->length; i++) {
    Token token = t_array->data[i];
    if (token.token_type == TOKEN_TYPE_SEMICOLON) {
      if (parens_balance != 0) {
        Tokeniser src = token.source;
        fprintf(stderr, "%s:%zu:%zu: Parenthesis imbalance found\n", src.filename, src.line, src.col);
        return false;
      }
    }
    else if (token.token_type == TOKEN_TYPE_OPAREN) parens_balance++;
    else if (token.token_type == TOKEN_TYPE_CPAREN) parens_balance--;
    if (parens_balance < 0) {
      fprintf(stderr, "Parenthesis imbalance found\n");
      return false;
    }
  }

  size_t ind = 0;
  while (ind < t_array->length) {
    Token this_token = t_array->data[ind];
    if (this_token.token_type == TOKEN_TYPE_EOF) return true;
    assert(ind < t_array->length-1);
    switch (this_token.token_type) {
      case TOKEN_TYPE_KEYWORD: {
        KeyWords kw = this_token.as.kw_token.value;
        switch (kw) {
          case KEYWORD_LET: {
            if (t_array->length - ind < 5) {
              fprintf(stderr, "%s:%zu:%zu: ERROR: 'let' expression badly formed\n",
                      this_token.source.filename, this_token.source.line, this_token.source.col);
              return false;
            }
            if (
              t_array->data[ind+1].token_type!=TOKEN_TYPE_ID || 
              t_array->data[ind+2].token_type!=TOKEN_TYPE_COLON || 
              t_array->data[ind+3].token_type!=TOKEN_TYPE_ID || 
              t_array->data[ind+4].token_type!=TOKEN_TYPE_ASSIGNMENT
            ) {
              fprintf(stderr, "%s:%zu:%zu: ERROR: 'let' expression badly formed. Should be 'let name: type = expr'\n",
                      this_token.source.filename, this_token.source.line, this_token.source.col);
              return false;
            }
            ind += 5;
            while (t_array->data[ind].token_type != TOKEN_TYPE_SEMICOLON && t_array->data[ind].token_type != TOKEN_TYPE_EOF) {
              ind += 1;
            }
          } break;
          case KEYWORD_COUNT: {
            fprintf(stderr, "%s:%zu:%zu: Unknown keyword. This is a bug in the tokeniser.\n", this_token.source.filename, this_token.source.line, this_token.source.col);
            return false;
          }
        }
      } break;
      case TOKEN_TYPE_EOF: return true;
      case TOKEN_TYPE_SEMICOLON: {
        ind += 1;
        continue;
      }
      case TOKEN_TYPE_ID:
      case TOKEN_TYPE_UNKNOWN:
      case TOKEN_TYPE_FLOAT:
      case TOKEN_TYPE_INT:
      case TOKEN_TYPE_STRING:
      case TOKEN_TYPE_ASSIGNMENT:
      case TOKEN_TYPE_ADD:
      case TOKEN_TYPE_MULT:
      case TOKEN_TYPE_SUB:
      case TOKEN_TYPE_DIV:
      case TOKEN_TYPE_OPAREN:
      case TOKEN_TYPE_CPAREN:
      case TOKEN_TYPE_COLON:
      case TOKEN_TYPE_COMMA:
      case TOKEN_TYPE_POINT:
      case TOKEN_TYPE_QUOTEMARK:
      case TOKEN_TYPE_COUNT: {
        fprintf(stderr, "%s:%zu:%zu: Error\n", this_token.source.filename, this_token.source.line, this_token.source.col);
        return false;
      }
    }
  }

  return true;
}

