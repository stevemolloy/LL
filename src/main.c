#include <assert.h>
#include <stdio.h>
#include <strings.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "sdm_lib.h"

#define EXTERN
#include "ll_lib.h"

static sdm_arena_t main_arena = {0};
static sdm_arena_t *active_arena = &main_arena;

void *active_alloc(size_t size)              { return sdm_arena_alloc(active_arena, size); }
void *active_realloc(void *ptr, size_t size) { return sdm_arena_realloc(active_arena, ptr, size); }

int main(void) {
  variable_lib = NULL;
  builtin_func_sigs = define_builtin_funcs();

  char *input_filename = "small_example.ll";
  // char *input_filename = "example.ll";
  // char *input_filename = "type_example.ll";
  char *buffer = sdm_read_entire_file(input_filename);

  FileData src_file = (FileData) {
    .filename = sdm_cstr_as_sv(input_filename),
    .contents = sdm_cstr_as_sv(buffer),
  };

  TokenArray token_array = {0};
  if (!tokenise_input_file(&src_file, &token_array)) return 1;
  validate_token_array(&token_array);

  ASTNodeArray program = {0};
  SDM_ENSURE_ARRAY_MIN_CAP(program, 1024);
  while (token_array.index < token_array.length) {
    ASTNode *ast = parse_expression(&token_array);
    if (ast == NULL) break;

    Token *next = get_current_token(&token_array);
    if ((next == NULL) || (next->type != TOKEN_TYPE_SEMICOLON)) {
      exit_with_error(next, "Missing semicolon or closing brace on or before this line?");
    }

    SDM_ARRAY_PUSH(program, *ast);
    token_array.index++;
  }

  for (size_t i=0; i<program.length; i++) {
    printf("%zu: ", i);
    print_ast(&program.data[i]);
  }

  // FILE *sink = stdout;
  FILE *sink = fopen("transpiler_out/transpiled_file.c", "w");
  transpile_program_to_C(sink, program);
  if (sink != stdout) fclose(sink);

  shfree(variable_lib);
  hmfree(builtin_func_sigs);
  sdm_arena_free(&main_arena);

  return 0;
}

