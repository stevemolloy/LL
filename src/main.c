#include "bundle/sdm_lib.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>

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

  // char *input_filename = "small_example.ll";
  char *input_filename = "example.ll";
  // char *input_filename = "bracket_example.ll";
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

  errno = 0;
  char *transpiler_output_dir = "transpiler_out";
  char *transpiled_filename = "transpiled_file.c";
  char *full_transpiled_path = SDM_MALLOC(strlen(transpiler_output_dir) + 1 + strlen(transpiled_filename) + 1);
  sprintf(full_transpiled_path, "%s/%s", transpiler_output_dir, transpiled_filename);
  struct stat info;
  if ((stat(transpiler_output_dir, &info) != 0) || !S_ISDIR(info.st_mode)) {
    errno = 0;
    if (mkdir(transpiler_output_dir, 0755) < 0) {
      fprintf(stderr, "ERROR: Could not create %s due to %s\n", transpiler_output_dir, strerror(errno));
      exit(1);
    }
  }
  FILE *sink = fopen(full_transpiled_path, "w");
  if (sink == NULL) {
    fprintf(stderr, "ERROR: Could not open %s due to %s\n", transpiled_filename, strerror(errno));
    exit(1);
  }
  transpile_program_to_C(sink, program);
  if (sink != stdout) fclose(sink);

  char *files_to_copy[] = {
    "acc_elements.c",
    "acc_elements.h",
    "lib.c",
    "lib.h",
    "Makefile",
    "sdm_lib.c",
    "sdm_lib.h",
    "stb_ds.h",
  };
  size_t num_files_to_copy = sizeof(files_to_copy) / sizeof(files_to_copy[0]);
  for (size_t i=0; i<num_files_to_copy; i++) {
    char *src_directory = "src/bundle/";
    char *src_path = SDM_MALLOC(strlen(src_directory) + strlen(files_to_copy[i]) + 1);
    char *dst_path = SDM_MALLOC(strlen(transpiler_output_dir) + 1 + strlen(files_to_copy[i]) + 1);
    sprintf(src_path, "%s%s", src_directory, files_to_copy[i]);
    sprintf(dst_path, "%s/%s", transpiler_output_dir, files_to_copy[i]);

    FILE *src = fopen(src_path, "rb");
    if (src==NULL) {
      fprintf(stderr, "Could not open %s for copying: %s\n", src_path, strerror(errno));
      exit(1);
    }
    FILE *dst = fopen(dst_path, "wb");
    if (dst==NULL) {
      fprintf(stderr, "Could not open dst file for copying: %s\n", strerror(errno));
      exit(1);
    }
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes_read, dst);
    }

    fclose(src);
    fclose(dst);
  }

  shfree(variable_lib);
  hmfree(builtin_func_sigs);
  sdm_arena_free(&main_arena);

  return 0;
}

