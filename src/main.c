#include <assert.h>
#include <stdio.h>

#include "token_lib.h"

static sdm_arena_t main_arena = {0};
static sdm_arena_t *active_arena = &main_arena;

void *active_alloc(size_t size)              { return sdm_arena_alloc(active_arena, size); }
void *active_realloc(void *ptr, size_t size) { return sdm_arena_realloc(active_arena, ptr, size); }

int main(void) {
  TokenArray token_array = {0};

  char *input_filename = "examples/small_example.ll";
  // char *input_filename = "examples/example.ll";
  // char *input_filename = "examples/type_example.ll";

  char *buffer = sdm_read_entire_file(input_filename);
  Tokeniser tokeniser = {
    .filename = input_filename,
    .contents = sdm_cstr_as_sv(buffer),
    .col = 1,
    .line = 1,
    .index = 0,
  };

  tokenise_input_file(&tokeniser, &token_array);
  find_and_apply_keywords(&token_array);
  if (!validate_token_array(&token_array)) {
    fprintf(stderr, "Invalid token array. This is a bug in the tokeniser.\n");
    return 1;
  };

  print_token_array(token_array);
  printf("Found %zu tokens, %zu lines, and %zu characters in %s\n", 
         token_array.length, tokeniser.line, tokeniser.index, tokeniser.filename);

  sdm_arena_free(&main_arena);

  return 0;
}

