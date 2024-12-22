#include <assert.h>
#include <stdio.h>
#include <strings.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef STB_DS_IMPLEMENTATION

#include "sdm_lib.h"

#include "ll_lib.h"

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

static sdm_arena_t main_arena = {0};
static sdm_arena_t *active_arena = &main_arena;

void *active_alloc(size_t size) {
  return sdm_arena_alloc(active_arena, size);
}
void *active_realloc(void *ptr, size_t size) {
  return sdm_arena_realloc(active_arena, ptr, size);
}

int main(void) {
  const char *input_filename = "small_example.ll";
  char *buffer = sdm_read_entire_file(input_filename);

  sdm_string_view source = sdm_cstr_as_sv(buffer);

  TokenArray token_array = {0};
  SDM_ENSURE_ARRAY_MIN_CAP(token_array, 1024);
  if (!tokenise_input_file(&source, &token_array)) return 1;

  sdm_arena_free(&main_arena);

  return 0;
}


