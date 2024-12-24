#include <assert.h>
#include <stdio.h>
#include <strings.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef STB_DS_IMPLEMENTATION

#include "sdm_lib.h"

#define EXTERN
#include "ll_lib.h"

static sdm_arena_t main_arena = {0};
static sdm_arena_t *active_arena = &main_arena;

void *active_alloc(size_t size)              { return sdm_arena_alloc(active_arena, size); }
void *active_realloc(void *ptr, size_t size) { return sdm_arena_realloc(active_arena, ptr, size); }

void write_astnode_toC(FILE *sink, ASTNode *ast) {
  char *vartype_as_C[VAR_TYPE_COUNT] = {
    [VAR_TYPE_INT] = "int",
    [VAR_TYPE_FLOAT] = "double",
  };

  char *binop_as_C[BINOP_COUNT] = {
    [BINOP_ADD] = "+",
    [BINOP_SUB] = "-",
    [BINOP_MUL] = "*",
    [BINOP_DIV] = "/",
  };

  switch (ast->type) {
    case NODE_TYPE_VARINIT: {
      VariableInit var_init = ast->as.var_init;
      fprintf(sink, "%s ", vartype_as_C[var_init.init_type]);
      fprintf(sink, SDM_SV_F, SDM_SV_Vals(var_init.name));
      if (var_init.init_value != NULL) {
        fprintf(sink, " = ");
        write_astnode_toC(sink, var_init.init_value);
      }
      fprintf(sink, ";\n");
    } break;
    case NODE_TYPE_VARIABLE: {
      Variable variable = ast->as.variable;
      fprintf(sink, SDM_SV_F, SDM_SV_Vals(variable.name));
    } break;
    case NODE_TYPE_BINOP: {
      BinOp binop = ast->as.binop;
      fprintf(sink, "( ");
      write_astnode_toC(sink, binop.lhs);
      fprintf(sink, " %s ", binop_as_C[binop.type]);
      write_astnode_toC(sink, binop.rhs);
      fprintf(sink, " )");
    } break;
    case NODE_TYPE_LITERAL: {
      Literal literal = ast->as.literal;
      fprintf(sink, SDM_SV_F, SDM_SV_Vals(literal.value));
    } break;
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Unreachable. This is a bug in the C transpiler.\n");
      exit(1);
    }
  }
}

int main(void) {
  variable_lib = NULL;

  char *input_filename = "small_example.ll";
  char *buffer = sdm_read_entire_file(input_filename);

  FileData src_file = (FileData) {
    .filename = sdm_cstr_as_sv(input_filename),
    .contents = sdm_cstr_as_sv(buffer),
  };

  TokenArray token_array = {0};
  if (!tokenise_input_file(&src_file, &token_array)) return 1;

  ASTNodeArray program = {0};
  SDM_ENSURE_ARRAY_MIN_CAP(program, 1024);
  while (token_array.index < token_array.length) {
    ASTNode *ast = parse_expression(&token_array);
    if (ast == NULL) break;
    SDM_ARRAY_PUSH(program, *ast);
  }

  printf("%s\n", buffer);
  for (size_t i=0; i<program.length; i++) {
    printf("%zu: ", i);
    print_ast(&program.data[i]);
  }

  // FILE *sink = stdout;
  FILE *sink = fopen("compiled_file.c", "w");
  if (sink == stdout) {
    printf("\n\n");
  }
  fprintf(sink, "#include <stdio.h>\n");
  fprintf(sink, "\n");
  fprintf(sink, "int main(void) {\n");
  for (size_t i=0; i<program.length; i++) {
    fprintf(sink, "\t");
    write_astnode_toC(sink, &program.data[i]);
  }
  fprintf(sink, "\n");
  fprintf(sink, "\treturn 0;\n");
  fprintf(sink, "}\n");

  if (sink != stdout) {
    fclose(sink);
  }

  shfree(variable_lib);
  sdm_arena_free(&main_arena);

  return 0;
}


