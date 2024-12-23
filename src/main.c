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
  [TOKEN_TYPE_VARINIT]    = "TOKEN_TYPE_VARINIT",
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

typedef struct ASTNode ASTNode;

typedef enum {
  NODE_TYPE_BINOP,
  NODE_TYPE_LITERAL,
  NODE_TYPE_VARINIT,
  NODE_TYPE_COUNT,
} ASTNodeType;

char *astnode_type_strings[NODE_TYPE_COUNT] = {
  [NODE_TYPE_BINOP]   = "NODE_TYPE_BINOP",
  [NODE_TYPE_LITERAL] = "NODE_TYPE_LITERAL",
  [NODE_TYPE_VARINIT] = "NODE_TYPE_VARINIT",
};

typedef enum {
  VAR_TYPE_INT,
} VarType;

typedef enum {
  BINOP_ADD,
  BINOP_SUB,
  BINOP_MUL,
  BINOP_DIV,
} BinOpType;

typedef struct {
  BinOpType type;
  ASTNode *lhs;
  ASTNode *rhs;
} BinOp;

typedef struct {
  VarType type;
  sdm_string_view value;
} Literal;

typedef struct {
  sdm_string_view name;
  VarType init_type;
  ASTNode *init_value;
} VariableInit;

struct ASTNode {
  ASTNodeType type;
  union {
    BinOp binop;
    Literal literal;
    VariableInit var_init;
  } as;
};

typedef struct {
  size_t capacity;
  size_t length;
  struct ASTNode *data;
} ASTNodeArray;

bool advance_token_index(TokenArray *token_array) {
  token_array->index++;
  return token_array->index >= token_array->length;
}

Token *get_current_token(TokenArray *token_array) {
  if (token_array->index >= token_array->length) return NULL;
  return &token_array->data[token_array->index];
}

#define print_ast(ARGS) print_ast_((ARGS), 0)
void print_ast_(ASTNode *ast, size_t level) {
  if (ast->type == NODE_TYPE_BINOP) {
    for (size_t i=0; i<level; i++) printf("  ");
    char *op_str = "";
    if (ast->as.binop.type == BINOP_DIV) op_str = "/";
    else if (ast->as.binop.type == BINOP_MUL) op_str = "*";
    else if (ast->as.binop.type == BINOP_SUB) op_str = "-";
    else if (ast->as.binop.type == BINOP_ADD) op_str = "+";
    else exit(1);
    printf("BINOP %s\n", op_str);
    print_ast_(ast->as.binop.lhs, level+1);
    print_ast_(ast->as.binop.rhs, level+1);
  } else if (ast->type == NODE_TYPE_LITERAL) {
    for (size_t i=0; i<level; i++) printf("  ");
    printf("LITERAL "SDM_SV_F"\n", SDM_SV_Vals(ast->as.literal.value));
  } else if (ast->type == NODE_TYPE_VARINIT) {
    for (size_t i=0; i<level; i++) printf("  ");
    printf("VARIABLE DECL "SDM_SV_F"", SDM_SV_Vals(ast->as.var_init.name));
    if (ast->as.var_init.init_value) {
      printf(" = \n");
      print_ast_(ast->as.var_init.init_value, level + 1);
    } else {
      printf("\n");
    }
  } else {
    printf("No printing method defined for this node type: %s\n", astnode_type_strings[ast->type]);
  }
}

ASTNode *parse_expression_primary(TokenArray *token_array) {
  Token *next = get_current_token(token_array);
  if (next->type == TOKEN_TYPE_NUMBER) {
    token_array->index++;
    ASTNode *num = SDM_MALLOC(sizeof(ASTNode));
    num->type = NODE_TYPE_LITERAL;
    num->as.literal.type = VAR_TYPE_INT;
    num->as.literal.value = next->content;
    return num;
  } else if (next->type == TOKEN_TYPE_SEMICOLON) {
    return NULL;
  }
  fprintf(stderr, SDM_SV_F":%zu:%zu: Unexpected token '"SDM_SV_F"'\n", 
          SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
  exit(1);
}

ASTNode *parse_expression_multdiv(TokenArray *token_array) {
  ASTNode *lhs = parse_expression_primary(token_array);
  if (!lhs) return NULL;

  Token *next = get_current_token(token_array);
  while ((next != NULL) && ((next->type == TOKEN_TYPE_MULT) || (next->type == TOKEN_TYPE_DIV))) {
    token_array->index++;
    ASTNode *rhs = parse_expression_primary(token_array);
    ASTNode *mult = SDM_MALLOC(sizeof(ASTNode));
    *mult = (ASTNode) {
      .type = NODE_TYPE_BINOP,
      .as.binop.type = (next->type==TOKEN_TYPE_MULT) ? BINOP_MUL : BINOP_DIV,
      .as.binop.lhs = lhs,
      .as.binop.rhs = rhs,
    };
    lhs = mult;
    next = get_current_token(token_array);
  }
  return lhs;
}

ASTNode *parse_expression_plus_minus(TokenArray *token_array) {
  ASTNode *lhs = parse_expression_multdiv(token_array);
  if (!lhs) return NULL;
  Token *next = get_current_token(token_array);
  while ((next != NULL) && ((next->type == TOKEN_TYPE_ADD) || (next->type == TOKEN_TYPE_SUB))) {
    token_array->index++;
    ASTNode *rhs = parse_expression_multdiv(token_array);
    if (!rhs) return NULL;
    ASTNode *plus = SDM_MALLOC(sizeof(ASTNode));
    *plus = (ASTNode) {
      .type = NODE_TYPE_BINOP,
      .as.binop.type = (next->type==TOKEN_TYPE_ADD) ? BINOP_ADD : BINOP_SUB,
      .as.binop.lhs = lhs,
      .as.binop.rhs = rhs,
    };
    lhs = plus;
    next = get_current_token(token_array);
  }
  return lhs;
}

ASTNode *parse_expression(TokenArray *token_array) {
  Token *next = get_current_token(token_array);
  while ((next != NULL) && (next->type == TOKEN_TYPE_COMMENT)) {
    token_array->index++;
    next = get_current_token(token_array);
  }
  if (next == NULL) return NULL;

  if (next->type == TOKEN_TYPE_VARINIT) {
    ASTNode *expr_node = SDM_MALLOC(sizeof(ASTNode));
    expr_node->type = NODE_TYPE_VARINIT;

    token_array->index++;
    next = get_current_token(token_array);
    if ((next==NULL) || (next->type != TOKEN_TYPE_SYMBOL)) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: 'let' should be followed by the name of the variable",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    expr_node->as.var_init.name = next->content;

    token_array->index++;
    next = get_current_token(token_array);
    if ((next==NULL) || (next->type != TOKEN_TYPE_COLON)) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: The variable name in a 'let' expression should be followed by ':' and then the variable type",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    token_array->index++;
    next = get_current_token(token_array);
    if ((next==NULL) || (next->type != TOKEN_TYPE_SYMBOL)) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: The variable name in a 'let' expression should be followed by ':' and then the variable type",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    if (sdm_svncmp(next->content, "int") == 0) {
      expr_node->as.var_init.init_type = VAR_TYPE_INT;
    } else {
      fprintf(stderr, SDM_SV_F":%zu:%zu: '"SDM_SV_F"' is not a recognised type",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
      exit(1);
    }

    token_array->index++;
    next = get_current_token(token_array);
    if ((next==NULL) || ((next->type != TOKEN_TYPE_ASSIGNMENT) && next->type != TOKEN_TYPE_SEMICOLON)) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: A variable declaration should end with initialisation or terminate with a semicolon \n",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    if (next->type == TOKEN_TYPE_SEMICOLON) {
      return expr_node;
    }

    token_array->index++;
    next = get_current_token(token_array);
    if (next==NULL) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: A variable declaration should end with initialisation or terminate with a semicolon \n",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    expr_node->as.var_init.init_value = parse_expression_plus_minus(token_array);

    next = get_current_token(token_array);
    if (next->type != TOKEN_TYPE_SEMICOLON) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: Missing semicolon on or before this line?\n", 
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }
    token_array->index++;

    return expr_node;
  }

  ASTNode *expr_node = parse_expression_plus_minus(token_array);

  next = get_current_token(token_array);
  if (next->type != TOKEN_TYPE_SEMICOLON) {
    fprintf(stderr, SDM_SV_F":%zu:%zu: Missing semicolon on or before this line?\n", 
            SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
    exit(1);
  }
  token_array->index++;

  return expr_node;
}

int main(void) {
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

  sdm_arena_free(&main_arena);

  return 0;
}


