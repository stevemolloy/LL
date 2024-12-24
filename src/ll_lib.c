#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "ll_lib.h"
#include "sdm_lib.h"
#include "stb_ds.h"

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
      loc->col = 1;
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

bool tokenise_input_file(FileData *file_data, TokenArray *token_array) {
  Loc loc = {.line = 1};
  loc.filename = file_data->filename;
  sdm_string_view *file_contents = &file_data->contents;
  sv_trim_follow(file_contents, &loc);

  while (file_contents->length > 0) {
    Token token = {0};
    token.loc = loc;

    size_t len = starts_with_float(file_contents->data);
    size_t jump_len = 1;
    if (isalpha(file_contents->data[0]) && sdm_svncmp(*file_contents, "let") == 0) {
      token.type = TOKEN_TYPE_VARINIT;
      jump_len = 3;
    } else if (isalpha(file_contents->data[0])) {
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
    else if (file_contents->data[0] == '/') token.type = TOKEN_TYPE_DIV;
    else if (file_contents->data[0] == '+') token.type = TOKEN_TYPE_ADD;
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

Token *get_current_token(TokenArray *token_array) {
  if (token_array->index >= token_array->length) return NULL;
  return &token_array->data[token_array->index];
}

char *astnode_type_strings[NODE_TYPE_COUNT] = {
  [NODE_TYPE_BINOP]   = "NODE_TYPE_BINOP",
  [NODE_TYPE_LITERAL] = "NODE_TYPE_LITERAL",
  [NODE_TYPE_VARIABLE] = "NODE_TYPE_VARIABLE",
  [NODE_TYPE_VARINIT] = "NODE_TYPE_VARINIT",
};

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
  } else if (ast->type == NODE_TYPE_VARIABLE) {
    for (size_t i=0; i<level; i++) printf("  ");
    printf(SDM_SV_F"\n", SDM_SV_Vals(ast->as.variable.name));
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
  } else if (next->type == TOKEN_TYPE_SYMBOL) {
    token_array->index++;
    int i = shgeti(variable_lib, sdm_sv_to_cstr(next->content));
    if (i < 0) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: Undefined variable '"SDM_SV_F"'\n", 
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
      exit(1);
    }
    ASTNode *variable = SDM_MALLOC(sizeof(ASTNode));
    variable->type = NODE_TYPE_VARIABLE;
    variable->as.variable = (Variable){.name = next->content};
    return variable;
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

    VariableDefn new_var = {0};

    token_array->index++;
    next = get_current_token(token_array);
    if ((next==NULL) || (next->type != TOKEN_TYPE_SYMBOL)) {
      fprintf(stderr, SDM_SV_F":%zu:%zu: 'let' should be followed by the name of the variable",
              SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col);
      exit(1);
    }

    expr_node->as.var_init.name = next->content;

    new_var.key = sdm_sv_to_cstr(expr_node->as.var_init.name);

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
    } else if (sdm_svncmp(next->content, "float") == 0) {
      expr_node->as.var_init.init_type = VAR_TYPE_FLOAT;
    } else {
      fprintf(stderr, SDM_SV_F":%zu:%zu: '"SDM_SV_F"' is not a recognised type\n",
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
      token_array->index++;
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
    new_var.value = expr_node->as.var_init.init_value;
    shputs(variable_lib, new_var);

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

