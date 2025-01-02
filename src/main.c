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

// VarType get_binop_result_type(BinOp *binop) {
//   ASTNode *lhs = binop->lhs;
//   ASTNode *rhs = binop->rhs;
//
//   VarType lhs_type, rhs_type;
//   switch (lhs->type) {
//     case NODE_TYPE_BINOP: {
//       lhs_type = get_binop_result_type(&lhs->as.binop);
//     } break;
//     case NODE_TYPE_LITERAL: {
//       lhs_type = lhs->as.literal.type;
//     } break;
//     case NODE_TYPE_VARIABLE: {
//       lhs_type = lhs->as.variable
//     } break;
//     case NODE_TYPE_FUNCALL: {} break;
//     case NODE_TYPE_VARINIT:
//     case NODE_TYPE_COUNT: {
//       fprintf(stderr, "Reached the unreachable\n");
//       exit(1);
//     } break;
//   }
// }

void check_astnode(ASTNode *ast) {
  char *astnode_type_strings[] = {
    [NODE_TYPE_BINOP]    = "NODE_TYPE_BINOP",
    [NODE_TYPE_LITERAL]  = "NODE_TYPE_LITERAL",
    [NODE_TYPE_VARIABLE] = "NODE_TYPE_VARIABLE",
    [NODE_TYPE_VARINIT]  = "NODE_TYPE_VARINIT",
    [NODE_TYPE_FUNCALL]  = "NODE_TYPE_FUNCALL",
  };
  static_assert(sizeof(astnode_type_strings) / sizeof(astnode_type_strings[0]) == NODE_TYPE_COUNT,
                "You forgot to extend 'astnode_type_strings'\n");

  switch (ast->type) {
    case NODE_TYPE_BINOP:    {
      fprintf(stderr, "Unable to check nodes of type '%s'\n", astnode_type_strings[ast->type]);
      exit(1);
    } break;
    case NODE_TYPE_LITERAL:  {
      fprintf(stderr, "Unable to check nodes of type '%s'\n", astnode_type_strings[ast->type]);
      exit(1);
    } break;
    case NODE_TYPE_VARINIT:  {
      fprintf(stderr, "Unable to check nodes of type '%s'\n", astnode_type_strings[ast->type]);
      exit(1);
    } break;
    case NODE_TYPE_VARIABLE: {
      fprintf(stderr, "Unable to check nodes of type '%s'\n", astnode_type_strings[ast->type]);
      exit(1);
    } break;
    case NODE_TYPE_FUNCALL:  {
      fprintf(stderr, "Unable to check nodes of type '%s'\n", astnode_type_strings[ast->type]);
      exit(1);
    } break;
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Reached the unreachable. Bug in the AST generator");
      exit(1);
    }
  }
}

int main(void) {
  variable_lib = NULL;
  builtin_func_sigs = define_builtin_funcs();

  char *input_filename = "small_example.ll";
  // char *input_filename = "example.ll";
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

