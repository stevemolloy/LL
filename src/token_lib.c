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

char *get_current_parser_string(Parser parser) {
  return parser.contents.data + parser.index;
}

void advance_to_next_line(Parser *parser) {
  while (parser->contents.data[parser->index] != '\n') {
    parser->index++;
  }
  parser->index++;
  if (parser->index < parser->contents.length) {
    parser->line++;
    parser->col = 1;
  }
}

bool starts_with_comment(Parser parser) {
  const char *comment_marker = "//";
  return strncmp(get_current_parser_string(parser), comment_marker, strlen(comment_marker)) == 0;
}

size_t starts_with_float(Parser parser) {
  char *endptr;
  char *input = get_current_parser_string(parser);
  errno = 0;
  strtod(input, &endptr);
  if (errno != 0) return 0;
  size_t retval = endptr - input;
  return retval;
}

char parser_current_char(const Parser *parser) {
  return parser->contents.data[parser->index];
}

bool parser_isalpha(const Parser *parser) {
  return isalpha(parser_current_char(parser));
}

bool parser_is_id_char(const Parser *parser) {
  return 
    parser_isalpha(parser) || 
    parser_current_char(parser) == '_' || 
    isalnum(parser_current_char(parser));
}

Token get_next_token(Parser *parser) {
  parser_trim(parser);

  while (starts_with_comment(*parser)) {
    advance_to_next_line(parser);
    parser_trim(parser);
  };

  Token token = {0};
  memcpy(&token.source, parser, sizeof(*parser));
  size_t len; // Only valid for the float/int part of the code

  if (parser->index >= parser->contents.length) {
    token.token_type = TOKEN_TYPE_EOF;
  } else if ((len = starts_with_float(*parser)) > 0) {
    // Could be an integer or a float
    char *start_ptr = parser->contents.data + parser->index;
    char *end_ptr = start_ptr;
    strtol(start_ptr, &end_ptr, 10);
    if ((end_ptr - start_ptr) == (long)len) {
      token.token_type = TOKEN_TYPE_INT;
      token.as.int_token.value = atoi(start_ptr);
    } else {
      token.token_type = TOKEN_TYPE_FLOAT;
      token.as.float_token.value = atof(start_ptr);
    }
    parser->index += len;
  } else if (parser_isalpha(parser)) {
    size_t start_index = parser->index;
    while (parser_is_id_char(parser)) parser->index++;
    size_t len = parser->index - start_index;
    token.token_type = TOKEN_TYPE_ID;
    token.as.id_token.value = SDM_MALLOC((len+1) * sizeof(char));
    memcpy(token.as.id_token.value, &parser->contents.data[start_index], len);
  } else if (parser_current_char(parser) == ',') {
    token.token_type = TOKEN_TYPE_COMMA;
    parser->index += 1;
  } else if (parser_current_char(parser) == '.') {
    token.token_type = TOKEN_TYPE_POINT;
    parser->index += 1;
  } else if (parser_current_char(parser) == '+') {
    token.token_type = TOKEN_TYPE_ADD;
    parser->index += 1;
  } else if (parser_current_char(parser) == '-') {
    token.token_type = TOKEN_TYPE_SUB;
    parser->index += 1;
  } else if (parser_current_char(parser) == '=') {
    token.token_type = TOKEN_TYPE_ASSIGNMENT;
    parser->index += 1;
  } else if (parser_current_char(parser) == '/') {
    token.token_type = TOKEN_TYPE_DIV;
    parser->index += 1;
  } else if (parser_current_char(parser) == '*') {
    token.token_type = TOKEN_TYPE_MULT;
    parser->index += 1;
  } else if (parser_current_char(parser) == ':') {
    token.token_type = TOKEN_TYPE_COLON;
    parser->index += 1;
  } else if (parser_current_char(parser) == ';') {
    token.token_type = TOKEN_TYPE_SEMICOLON;
    parser->index += 1;
  } else if (parser_current_char(parser) == '(') {
    token.token_type = TOKEN_TYPE_OPAREN;
    parser->index += 1;
  } else if (parser_current_char(parser) == ')') {
    token.token_type = TOKEN_TYPE_CPAREN;
    parser->index += 1;
  } else if (parser_current_char(parser) == '"') {
    // We have a string.  We have to find the end
    parser->index++;
    size_t str_start = parser->index;
    while ((parser->index < parser->contents.length) && (parser_current_char(parser) != '"')) {
      parser->index++;
    }
    size_t str_len = parser->index - str_start;
    token.token_type = TOKEN_TYPE_STRING;
    token.as.str_token.value = SDM_MALLOC(str_len + 1);
    memset(token.as.str_token.value, 0, str_len + 1);
    memcpy(token.as.str_token.value, parser->contents.data+str_start, str_len);
    parser->index += str_len + 1;
  } else {
    fprintf(stderr, "WARNING: Unsure how to parse '%c'\n", parser->contents.data[parser->index]);
    token.token_type = TOKEN_TYPE_UNKNOWN;
    parser->index += 1;
  }

  return token;
}

void tokenise_input_file(Parser *parser, TokenArray *token_array) {
  sdm_string_view contents = parser->contents;

  while (parser->index < contents.length) {
    SDM_ARRAY_PUSH(*token_array, get_next_token(parser));
  }
}

void parser_trim(Parser *parser) {
  char *text = get_current_parser_string(*parser);
  while (strlen(text) > 0 && isspace(*text)) {
    if (*text == '\n') {
      parser->line++;
      parser->col = 1;
    } else {
      parser->col++;
    }
    parser->index++;
    text = get_current_parser_string(*parser);
  }
}

void parser_chop(Parser *parser, size_t len) {
  sdm_string_view *SV = &parser->contents;
  for (size_t i=0; SV->length>0 && i<len; i++) {
    if (SV->data[0] == '\n') {
      parser->line++;
      parser->col = 0;
    } else {
      parser->col++;
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
        Parser src = token.source;
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

