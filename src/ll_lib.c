#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "ll_lib.h"
#include "sdm_lib.h"
#include "stb_ds.h"

Token *get_next_token(TokenArray *token_array) {
  token_array->index++;
  if (token_array->index >= token_array->length)
    return NULL;
  return &token_array->data[token_array->index];
}

void expect_next_token_type(TokenArray *token_array, TokenType type, char *err_mesg) {
  Token *token = get_next_token(token_array);
  if ((token == NULL) || (token->type != type)) {
    exit_with_error(token, err_mesg);
  }
}

void validate_token_array(TokenArray *token_array) {
  Token *token;
  while (token_array->index < token_array->length) {
    token = get_current_token(token_array);
    while ((token != NULL) && (token_array->index < token_array->length) && (token->type == TOKEN_TYPE_COMMENT)) {
      token = get_next_token(token_array);
      continue;
    }
    if (token == NULL) break;
    if (token->type == TOKEN_TYPE_VARINIT) {
      // Check that variable initialisation can proceed correctly
      expect_next_token_type(token_array, TOKEN_TYPE_SYMBOL, "The 'let' keyword should be followed by the name of the variable");
      expect_next_token_type(token_array, TOKEN_TYPE_COLON, "The name of the variable in a 'let' expression should be followed by a ':' and then the type of the variable");
      expect_next_token_type(token_array, TOKEN_TYPE_SYMBOL, "The name of the variable in a 'let' expression should be followed by a ':' and then the type of the variable");
      token = get_next_token(token_array);
      if ((token != NULL) && (token->type == TOKEN_TYPE_SEMICOLON)) {
        // Declaration with no initialisation
        token_array->index++;
        continue;
      }
      token_array->index--;
      expect_next_token_type(token_array, TOKEN_TYPE_ASSIGNMENT, "A variable declaration must be followed by a terminating semicolon, or by '=' and the variable initialisation");
      token = get_next_token(token_array);
      if ((token == NULL) || (token->type == TOKEN_TYPE_SEMICOLON)) {
        exit_with_error(token, "Variable assignment RHS is empty");
      }
      token = get_next_token(token_array);
      while ((token != NULL) && (token->type != TOKEN_TYPE_SEMICOLON)) {
        token = get_next_token(token_array);
      }
      token_array->index++;
    } else if (token->type == TOKEN_TYPE_SYMBOL) {
      // Variable reassignment?
      // Calling a function?
      token = get_next_token(token_array);
      if (token->type == TOKEN_TYPE_ASSIGNMENT) {
        // Variable reassignment
        token = get_next_token(token_array);
        if ((token == NULL) || (token->type == TOKEN_TYPE_SEMICOLON)) {
          exit_with_error(token, "Variable reassignment missing a RHS");
        }
        token = get_next_token(token_array);
        while ((token != NULL) && (token->type != TOKEN_TYPE_SEMICOLON)) {
          token = get_next_token(token_array);
        }
        token_array->index++;
      } else if (token->type == TOKEN_TYPE_OPAREN) {
        token = get_next_token(token_array);
        while ((token != NULL) && (token->type != TOKEN_TYPE_SEMICOLON)) {
          token = get_next_token(token_array);
        }
        token_array->index++;
      }
    } else {
      token = get_current_token(token_array);
      exit_with_error(token, "No idea what to do with this");
    }
  }
  token_array->index = 0;
}

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

char *token_strings[] = {
  [TOKEN_TYPE_SYMBOL]     = "TOKEN_TYPE_SYMBOL",
  [TOKEN_TYPE_VARINIT]    = "TOKEN_TYPE_VARINIT",
  [TOKEN_TYPE_NUMBER]     = "TOKEN_TYPE_NUMBER",
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
  [TOKEN_TYPE_COMMENT]    = "TOKEN_TYPE_COMMENT",
  [TOKEN_TYPE_POINT]      = "TOKEN_TYPE_POINT",
  [TOKEN_TYPE_STRING]     = "TOKEN_TYPE_STRING",
};
static_assert(sizeof(token_strings) / sizeof(token_strings[0]) == TOKEN_TYPE_COUNT, "You forgot to extend the TOKEN_TYPE array");

char *astnode_type_strings[NODE_TYPE_COUNT] = {
  [NODE_TYPE_BINOP]    = "NODE_TYPE_BINOP",
  [NODE_TYPE_LITERAL]  = "NODE_TYPE_LITERAL",
  [NODE_TYPE_VARIABLE] = "NODE_TYPE_VARIABLE",
  [NODE_TYPE_VARINIT]  = "NODE_TYPE_VARINIT",
  [NODE_TYPE_FUNCALL]  = "NODE_TYPE_FUNCALL",
};

char *binop_type_strings[] = {
  [BINOP_ADD] = "BINOP_ADD",
  [BINOP_SUB] = "BINOP_SUB",
  [BINOP_MUL] = "BINOP_MUL",
  [BINOP_DIV] = "BINOP_DIV",
};
static_assert(sizeof(binop_type_strings) / sizeof(binop_type_strings[0]) == BINOP_COUNT, "You forgot to extend the BINOP_TYPE string array");

char *var_type_strings[] = {
  [VAR_TYPE_VOID]    = "VAR_TYPE_VOID",
  [VAR_TYPE_INT]     = "VAR_TYPE_INT",
  [VAR_TYPE_FLOAT]   = "VAR_TYPE_FLOAT",
  [VAR_TYPE_STRING]  = "VAR_TYPE_STRING",
  [VAR_TYPE_ELEMENT] = "VAR_TYPE_ELEMENT",
  [VAR_TYPE_LINE]    = "VAR_TYPE_LINE",
};
static_assert(sizeof(var_type_strings) / sizeof(var_type_strings[0]) == VAR_TYPE_COUNT, "You forgot to extend var_type_strings");

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
      jump_len++;
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
    else if (file_contents->data[0] == '.') token.type = TOKEN_TYPE_POINT;
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

Token *peek_next_token(TokenArray *token_array) {
  size_t index = token_array->index + 1;
  if (index >= token_array->length) return NULL;
  return &token_array->data[index];
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
  } else if (ast->type == NODE_TYPE_VARIABLE) {
    for (size_t i=0; i<level; i++) printf("  ");
    printf(SDM_SV_F"\n", SDM_SV_Vals(ast->as.variable.name));
  } else if (ast->type == NODE_TYPE_FUNCALL) {
    for (size_t i=0; i<level; i++) printf("  ");
    printf("CALL "SDM_SV_F"\n", SDM_SV_Vals(ast->as.funcall.name));
    for (size_t i=0; i<ast->as.funcall.args->length; i++) {
      print_ast_(&ast->as.funcall.args->data[i], level+1);
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
    num->as.literal.value = next->content;
    char *start_ptr = next->content.data;
    char *endptr = start_ptr;
    strtol(start_ptr, &endptr, 10);
    if ((endptr - start_ptr) == (long)next->content.length) {
      num->result_type = VAR_TYPE_INT;
      num->as.literal.type = VAR_TYPE_INT;
    } else {
      num->result_type = VAR_TYPE_FLOAT;
      num->as.literal.type = VAR_TYPE_FLOAT;
    }
    return num;
   } else if (next->type == TOKEN_TYPE_OPAREN) {
     token_array->index++;
     ASTNode *paren_expr = parse_expression(token_array);
     if (token_array->data[token_array->index].type != TOKEN_TYPE_CPAREN) {
       fprintf(stderr, "Unterminated paren expression\n");
       exit(1);
     }
     token_array->index++;
     return paren_expr;
  } else if (next->type == TOKEN_TYPE_SUB) {
    token_array->index++;
    ASTNode *subnode = SDM_MALLOC(sizeof(ASTNode));
    subnode->type = NODE_TYPE_BINOP;
    subnode->as.binop.type = BINOP_MUL;
    subnode->as.binop.lhs = SDM_MALLOC(sizeof(ASTNode));
    subnode->as.binop.lhs->type = NODE_TYPE_LITERAL;
    subnode->as.binop.lhs->as.literal.type = VAR_TYPE_INT;
    subnode->as.binop.lhs->result_type = VAR_TYPE_INT;
    subnode->as.binop.lhs->as.literal.value = sdm_cstr_as_sv("-1");

    subnode->as.binop.rhs = parse_expression_primary(token_array);
    subnode->result_type = subnode->as.binop.rhs->result_type;
    subnode->as.binop.result_type = subnode->as.binop.rhs->result_type;

    return subnode;
  } else if (next->type == TOKEN_TYPE_STRING) {
    token_array->index++;
    ASTNode *stringnode = SDM_MALLOC(sizeof(ASTNode));
    stringnode->type = NODE_TYPE_LITERAL;
    stringnode->result_type = VAR_TYPE_STRING;
    stringnode->as.literal.value = next->content;
    stringnode->as.literal.type = VAR_TYPE_STRING;
    return stringnode;
  } else if (next->type == TOKEN_TYPE_SYMBOL) {
    // Might be a variable or a function call
    sdm_string_view name = next->content;
    token_array->index++;
    if ((token_array->index < (token_array->length)) && (get_current_token(token_array)->type == TOKEN_TYPE_OPAREN)) {
      // This is a function call
      ASTNode *funcall = SDM_MALLOC(sizeof(ASTNode));
      funcall->type = NODE_TYPE_FUNCALL;
      funcall->as.funcall.name  = name;
      int index = shgeti(builtin_func_sigs, sdm_sv_to_cstr(name));
      if (index < 0) {
        token_array->index--;
        exit_with_error(get_current_token(token_array), "Calling an undefined function.");
      }
      funcall->result_type = builtin_func_sigs[index].value.return_type;
      token_array->index++;
      funcall->as.funcall.args = SDM_MALLOC(sizeof(ASTNodeArray));
      funcall->as.funcall.named_args = SDM_MALLOC(sizeof(VariableInitArray));
      do {
        next = get_current_token((token_array));
        Token *peek = peek_next_token(token_array);
        if ((peek != NULL) && (peek->type == TOKEN_TYPE_ASSIGNMENT) && (next != NULL) && (next->type == TOKEN_TYPE_SYMBOL)) {
          VariableInit named_argument = {0};
          named_argument.name = next->content;
          token_array->index += 2;
          named_argument.init_value = parse_expression(token_array);
          SDM_ARRAY_PUSH(*funcall->as.funcall.named_args, named_argument);
        } else {
          SDM_ARRAY_PUSH(*funcall->as.funcall.args, *parse_expression(token_array));
        }
        next = get_current_token(token_array);
        if (next->type == TOKEN_TYPE_COMMA) {
          token_array->index++;
        }
      } while (next->type != TOKEN_TYPE_CPAREN);
      token_array->index++;
      return funcall;
    } else {
      // This is a variable
      int i = shgeti(variable_lib, sdm_sv_to_cstr(name));
      if (i < 0) {
        fprintf(stderr, SDM_SV_F":%zu:%zu: Undefined variable '"SDM_SV_F"'\n", 
                SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
        exit(1);
      }
      ASTNode *variable = SDM_MALLOC(sizeof(ASTNode));
      variable->type = NODE_TYPE_VARIABLE;
      variable->as.variable = (Variable){.name = next->content};
      variable->result_type = variable_lib[i].value->result_type;
      return variable;
    }
  } else if (next->type == TOKEN_TYPE_SEMICOLON) {
    return NULL;
  }
  // fprintf(stderr, SDM_SV_F":%zu:%zu: Unexpected token '"SDM_SV_F"'\n", 
  //         SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
  // exit(1);
  return NULL;
}

VarType get_astnode_type(ASTNode *ast) {
  switch (ast->type) {
    case NODE_TYPE_BINOP:    return ast->as.binop.result_type;
    case NODE_TYPE_LITERAL:  return ast->as.literal.type;
    case NODE_TYPE_VARIABLE: {
      sdm_string_view varname = ast->as.variable.name;
      int index = shgeti(variable_lib, sdm_sv_to_cstr(varname));
      if (index < 0) {
        fprintf(stderr, "Use of unknown variable '"SDM_SV_F"'. This should have been caught earlier in the compilation.\n", SDM_SV_Vals(varname));
        exit(1);
      }
      return get_astnode_type(variable_lib[index].value);
    } break;
    case NODE_TYPE_FUNCALL: {
      return ast->result_type;
    } break;
    case NODE_TYPE_VARINIT:
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Unreachable. You have found a bug in the type system.\n");
      exit(1);
    } break;
  }
  fprintf(stderr, "Unreachable. You have found a bug in the type system.\n");
  exit(1);
}

VarType calc_binop_resulttype(BinOp *binop) {
  ASTNode *lhs = binop->lhs;
  ASTNode *rhs = binop->rhs;

  VarType lhs_type = get_astnode_type(lhs);
  VarType rhs_type = get_astnode_type(rhs);

  if (binop->type == BINOP_MUL) {
    if ((lhs_type == VAR_TYPE_FLOAT) || (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_FLOAT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_INT;
    } else if ((lhs_type = VAR_TYPE_INT) && (rhs_type = VAR_TYPE_LINE)) {
      return VAR_TYPE_LINE;
    } else if ((lhs_type = VAR_TYPE_LINE) && (rhs_type = VAR_TYPE_INT)) {
      return VAR_TYPE_LINE;
    } else if ((lhs_type = VAR_TYPE_INT) && (rhs_type = VAR_TYPE_ELEMENT)) {
      return VAR_TYPE_ELEMENT;
    } else if ((lhs_type = VAR_TYPE_ELEMENT) && (rhs_type = VAR_TYPE_INT)) {
      return VAR_TYPE_LINE;
    }
  } else if (binop->type == BINOP_DIV) {
    if ((lhs_type == VAR_TYPE_FLOAT) || (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_FLOAT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_INT;
    }
  } else if ((binop->type == BINOP_ADD) || (binop->type == BINOP_SUB)) {
    if ((lhs_type == VAR_TYPE_FLOAT) || (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_FLOAT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_FLOAT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_FLOAT;
    } else if ((lhs_type == VAR_TYPE_INT) && (rhs_type == VAR_TYPE_INT)) {
      return VAR_TYPE_INT;
    } else if ((lhs_type == VAR_TYPE_ELEMENT) && (rhs_type == VAR_TYPE_ELEMENT)) {
      return VAR_TYPE_LINE;
    } else if ((lhs_type == VAR_TYPE_LINE) && (rhs_type == VAR_TYPE_ELEMENT)) {
      return VAR_TYPE_LINE;
    } else if ((lhs_type == VAR_TYPE_ELEMENT) && (rhs_type == VAR_TYPE_LINE)) {
      return VAR_TYPE_LINE;
    } else if ((lhs_type == VAR_TYPE_LINE) && (rhs_type == VAR_TYPE_LINE)) {
      return VAR_TYPE_LINE;
    }
  }
  fprintf(stderr, "I don't know how to combine variables of type '%s' and '%s' with '%s'\n", var_type_strings[lhs_type], var_type_strings[rhs_type], binop_type_strings[binop->type]);
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
    mult->as.binop.result_type = calc_binop_resulttype(&mult->as.binop);
    mult->result_type = mult->as.binop.result_type;
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
    plus->as.binop.result_type = calc_binop_resulttype(&plus->as.binop);
    plus->result_type = plus->as.binop.result_type;
    lhs = plus;
    next = get_current_token(token_array);
  }
  return lhs;
}

ASTNode *parse_variable_initiation(TokenArray *token_array) {
  Token *next = get_current_token(token_array);

  ASTNode *expr_node = SDM_MALLOC(sizeof(ASTNode));
  expr_node->type = NODE_TYPE_VARINIT;

  VariableDefn new_var = {0};

  token_array->index++;
  next = get_current_token(token_array);
  if ((next==NULL) || (next->type != TOKEN_TYPE_SYMBOL)) {
    exit_with_error(next, "'let' should be followed by the name of the variable");
  }

  expr_node->as.var_init.name = next->content;

  new_var.key = sdm_sv_to_cstr(expr_node->as.var_init.name);

  token_array->index++;
  next = get_current_token(token_array);
  if ((next==NULL) || (next->type != TOKEN_TYPE_COLON)) {
    exit_with_error(next, "The variable name in a 'let' expression should be followed by ':' and then the variable type");
  }

  token_array->index++;
  next = get_current_token(token_array);
  if ((next==NULL) || (next->type != TOKEN_TYPE_SYMBOL)) {
    exit_with_error(next, "The variable name in a 'let' expression should be followed by ':' and then the variable type");
  }

  if (sdm_svncmp(next->content, "int") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_INT;
  } else if (sdm_svncmp(next->content, "float") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_FLOAT;
  } else if (sdm_svncmp(next->content, "Drift") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Quad") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Marker") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Bend") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Sextupole") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Octupole") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Cavity") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_ELEMENT;
  } else if (sdm_svncmp(next->content, "Line") == 0) {
    expr_node->as.var_init.init_type = VAR_TYPE_LINE;
  } else {
    fprintf(stderr, SDM_SV_F":%zu:%zu: '"SDM_SV_F"' is not a recognised type\n",
            SDM_SV_Vals(next->loc.filename), next->loc.line, next->loc.col, SDM_SV_Vals(next->content));
    exit(1);
  }

  token_array->index++;
  next = get_current_token(token_array);
  if ((next==NULL) || ((next->type != TOKEN_TYPE_ASSIGNMENT) && next->type != TOKEN_TYPE_SEMICOLON)) {
    exit_with_error(next, "A variable declaration should end with initialisation or terminate with a semicolon");
  }

  if (next->type == TOKEN_TYPE_SEMICOLON) {
    return expr_node;
  }

  token_array->index++;
  next = get_current_token(token_array);
  if (next==NULL) {
    exit_with_error(next, "A variable declaration should end with initialisation or terminate with a semicolon");
  }

  expr_node->as.var_init.init_value = parse_expression_plus_minus(token_array);
  new_var.value = expr_node->as.var_init.init_value;
  switch (new_var.value->type) {
    case NODE_TYPE_BINOP: {
      new_var.value->as.binop.result_type = expr_node->as.var_init.init_type;
      new_var.value->result_type = expr_node->as.var_init.init_type;
    } break;
    case NODE_TYPE_LITERAL: {
      new_var.value->as.literal.type = expr_node->as.var_init.init_type;
      new_var.value->result_type = expr_node->as.var_init.init_type;
    } break;
    case NODE_TYPE_FUNCALL: {
      // Nothing to do.
    } break;
    case NODE_TYPE_VARINIT: {
      exit_with_error(next, "You can't instantiate variables inside a variable instantiation!");
    } break;
    case NODE_TYPE_VARIABLE:
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Bug in variable init code\n");
      exit(1);
    } break;
  }
  shputs(variable_lib, new_var);

  return expr_node;
}

ASTNode *parse_expression(TokenArray *token_array) {
  Token *next = get_current_token(token_array);
  while ((next != NULL) && (next->type == TOKEN_TYPE_COMMENT)) {
    token_array->index++;
    next = get_current_token(token_array);
  }

  if (next == NULL) return NULL;
  if (next->type == TOKEN_TYPE_VARINIT) return parse_variable_initiation(token_array);
  ASTNode *expr_node = parse_expression_plus_minus(token_array);

  return expr_node;
}

ASTNode *get_named_variable_value(VariableInitArray *named_args, char *name) {
  for (size_t i=0; i<named_args->length; i++) {
    if (sdm_svncmp(named_args->data[i].name, name) == 0) return named_args->data[i].init_value;
  }
  return NULL;
}

void write_variable_defn(FILE *sink, ASTNode *var) {
  if (var == NULL) {
    fprintf(sink, "0.0");
    return;
  }
  if (var->type == NODE_TYPE_VARIABLE) {
    fprintf(sink, SDM_SV_F, SDM_SV_Vals(var->as.variable.name));
  } else if (var->type == NODE_TYPE_LITERAL) {
    fprintf(sink, SDM_SV_F, SDM_SV_Vals(var->as.literal.value));
  } else if (var->type == NODE_TYPE_BINOP) {
    write_astnode_toC(sink, var);
  } else {
    fprintf(stderr, "Unable to transpile a variable of type '%s'\n", astnode_type_strings[var->type]);
    exit(1);
  }
}

int evaluate_int_astnode(ASTNode *node){
  int value;

  switch (node->type) {
    case NODE_TYPE_LITERAL: {
      Literal literal = node->as.literal;
      value = atoi(literal.value.data);
    } break;
    case NODE_TYPE_VARIABLE: {
      int index = shgeti(variable_lib, sdm_sv_to_cstr(node->as.variable.name));
      if (index < 0) {
        fprintf(stderr, "Attempted use of undefined variable, '"SDM_SV_F"'\n", SDM_SV_Vals(node->as.variable.name));
        exit(1);
      }
      value = evaluate_int_astnode(variable_lib[index].value);
    } break;
    case NODE_TYPE_BINOP: {
      int lhs = evaluate_int_astnode(node->as.binop.lhs);
      int rhs = evaluate_int_astnode(node->as.binop.rhs);
      switch (node->as.binop.type) {
        case BINOP_ADD: { value = rhs + lhs; } break;
        case BINOP_SUB: { value = rhs - lhs; } break;
        case BINOP_MUL: { value = rhs * lhs; } break;
        case BINOP_DIV: { value = rhs / lhs; } break;
        case BINOP_COUNT: {
          fprintf(stderr, "Something has gone badly wrong evaluating a binary operation\n");
          exit(1);
        } break;
      }
    } break;
    case NODE_TYPE_FUNCALL: {
      fprintf(stderr, "Unable to evaluate function calls at this point\n");
      exit(1);
    } break;
    case NODE_TYPE_VARINIT:
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Something has gone badly wrong evaluating an ASTNode\n");
      exit(1);
    }
  }
  return value;
}

void write_astnode_toC(FILE *sink, ASTNode *ast) {
  char *vartype_as_C[] = {
    [VAR_TYPE_VOID]    = "void",
    [VAR_TYPE_INT]     = "int",
    [VAR_TYPE_FLOAT]   = "double",
    [VAR_TYPE_ELEMENT] = "Element",
    [VAR_TYPE_LINE]    = "Line",
  };
  static_assert(sizeof(vartype_as_C)/sizeof(vartype_as_C[0]) == VAR_TYPE_COUNT, "Number of variable types does not match number of C declarations");

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
      if ((binop.type == BINOP_ADD) && (binop.lhs->result_type == VAR_TYPE_ELEMENT) && (binop.rhs->result_type == VAR_TYPE_ELEMENT)) {
        fprintf(sink, "concat_two_elements(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_ADD) && (binop.lhs->result_type == VAR_TYPE_LINE) && (binop.rhs->result_type == VAR_TYPE_ELEMENT)) {
        fprintf(sink, "add_element_to_line(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_ADD) && (binop.lhs->result_type == VAR_TYPE_ELEMENT) && (binop.rhs->result_type == VAR_TYPE_LINE)) {
        fprintf(sink, "add_line_to_element(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_ADD) && (binop.lhs->result_type == VAR_TYPE_LINE) && (binop.rhs->result_type == VAR_TYPE_LINE)) {
        fprintf(sink, "add_line_to_line(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_SUB) && (binop.lhs->result_type == VAR_TYPE_ELEMENT) && (binop.rhs->result_type == VAR_TYPE_LINE)) {
        fprintf(sink, "add_reversedline_to_element(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_SUB) && (binop.lhs->result_type == VAR_TYPE_LINE) && (binop.rhs->result_type == VAR_TYPE_LINE)) {
        fprintf(sink, "add_reversedline_to_line(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_MUL) && (binop.lhs->result_type == VAR_TYPE_INT) && (binop.rhs->result_type == VAR_TYPE_LINE)) {
        fprintf(sink, "int_times_line(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_MUL) && (binop.lhs->result_type == VAR_TYPE_LINE) && (binop.rhs->result_type == VAR_TYPE_INT)) {
        fprintf(sink, "line_times_int(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
      } else if ((binop.type == BINOP_MUL) && (binop.lhs->result_type == VAR_TYPE_INT) && (binop.rhs->result_type == VAR_TYPE_ELEMENT)) {
        fprintf(sink, "int_times_element(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else if ((binop.type == BINOP_MUL) && (binop.lhs->result_type == VAR_TYPE_ELEMENT) && (binop.rhs->result_type == VAR_TYPE_INT)) {
        fprintf(sink, "element_times_int(");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, ", ");
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, ")");
      } else {
        fprintf(sink, "( ");
        write_astnode_toC(sink, binop.lhs);
        fprintf(sink, " %s ", binop_as_C[binop.type]);
        write_astnode_toC(sink, binop.rhs);
        fprintf(sink, " )");
      }
    } break;
    case NODE_TYPE_LITERAL: {
      Literal literal = ast->as.literal;
      fprintf(sink, SDM_SV_F, SDM_SV_Vals(literal.value));
    } break;
    case NODE_TYPE_FUNCALL: {
      FunCall funcall = ast->as.funcall;
      if (sdm_svncmp(funcall.name, "println") == 0) {
        fprintf(sink, "printf(\"");

        for (size_t i=0; i<funcall.args->length; i++) {
          VarType arg_type = get_astnode_type(&funcall.args->data[i]);
          if (i != 0) fprintf(sink, " ");
          if (arg_type == VAR_TYPE_INT) fprintf(sink, "%%d");
          else if (arg_type == VAR_TYPE_FLOAT) fprintf(sink, "%%f");
          else if (arg_type == VAR_TYPE_STRING) fprintf(sink, "%%s");
          else fprintf(stderr, "Not sure how to print a variable of type '%s'\n", var_type_strings[arg_type]);
        }

        fprintf(sink, "\\n\", ");
        for (size_t i=0; i<funcall.args->length; i++) {
          if (funcall.args->data[i].type == NODE_TYPE_VARIABLE) {
            fprintf(sink, SDM_SV_F, SDM_SV_Vals(funcall.args->data[i].as.variable.name));
          } else if (funcall.args->data[i].type == NODE_TYPE_LITERAL) {
            fprintf(sink, SDM_SV_F, SDM_SV_Vals(funcall.args->data[i].as.literal.value));
          }
          if (i != funcall.args->length-1) fprintf(sink, ", ");
        }
        fprintf(sink, ");\n");
      } else if (sdm_svncmp(funcall.name, "Line") == 0) {
        fprintf(sink, "create_line_with_elements(");
        for (size_t i=0; i<funcall.args->length; i++) {
          if (funcall.args->data[i].type == NODE_TYPE_VARIABLE) {
            Variable argument = funcall.args->data[i].as.variable;
            int index = shgeti(variable_lib, sdm_sv_to_cstr(argument.name));
            if (index < 0) {
              fprintf(stderr, "Unknown variable, '"SDM_SV_F"'\n", SDM_SV_Vals(argument.name));
              exit(1);
            }
            ASTNode *variable = variable_lib[index].value;
            if (variable->result_type == VAR_TYPE_ELEMENT) {
              fprintf(sink, "ARG_ELEMENT, ");
            } else if (variable->result_type == VAR_TYPE_LINE) {
              fprintf(sink, "ARG_LINE, ");
            } else {
              fprintf(stderr, "Wrong type of variable\n");
              exit(1);
            }
            fprintf(sink, SDM_SV_F, SDM_SV_Vals(argument.name));
          } else if (funcall.args->data[i].type == NODE_TYPE_BINOP) {
            BinOp op = funcall.args->data[i].as.binop;
            if (op.lhs->result_type == VAR_TYPE_INT && op.rhs->result_type == VAR_TYPE_ELEMENT) {
              int lhs = evaluate_int_astnode(op.lhs);
              for (int i=0; i<abs(lhs); i++) {
                fprintf(sink, "ARG_ELEMENT, ");
                fprintf(sink, SDM_SV_F, SDM_SV_Vals(op.rhs->as.variable.name));
                if (i != abs(lhs)-1) fprintf(sink, ", ");
              }
            } else if (op.lhs->result_type == VAR_TYPE_ELEMENT && op.rhs->result_type == VAR_TYPE_INT ) {
              int rhs = evaluate_int_astnode(op.lhs);
              for (int i=0; i<abs(rhs); i++) {
                fprintf(sink, "ARG_ELEMENT, ");
                fprintf(sink, SDM_SV_F, SDM_SV_Vals(op.lhs->as.variable.name));
                if (i != abs(rhs)-1) fprintf(sink, ", ");
              }
            } else if (op.lhs->result_type == VAR_TYPE_INT && op.rhs->result_type == VAR_TYPE_LINE) {
              int lhs = evaluate_int_astnode(op.lhs);
              if (lhs > 0) {
                for (int i=0; i<lhs; i++) {
                  fprintf(sink, "ARG_LINE, ");
                  fprintf(sink, SDM_SV_F, SDM_SV_Vals(op.rhs->as.variable.name));
                  if (i != abs(lhs)-1) fprintf(sink, ", ");
                }
              } else {
                for (int i=0; i<abs(lhs); i++) {
                  fprintf(sink, "ARG_LINE, ");
                  fprintf(sink, "reverse_line("SDM_SV_F")", SDM_SV_Vals(op.rhs->as.variable.name));
                  if (i != abs(lhs)-1) fprintf(sink, ", ");
                }
              }
            }
          } else {
            fprintf(stderr, "uh oh!\n");
            exit(1);
          }
          if (i != funcall.args->length-1) fprintf(sink, ", ");
        };
        fprintf(sink, ", -1)");
      } else if (sdm_svncmp(funcall.name, "Drift") == 0) {
        fprintf(sink, "make_drift(");
        ASTNode *len_var = funcall.named_args->data[0].init_value;
        write_variable_defn(sink, len_var); fprintf(sink, ")");
      } else if (sdm_svncmp(funcall.name, "Quad") == 0) {
        fprintf(sink, "make_quad(");
        ASTNode *len_var = get_named_variable_value(funcall.named_args, "L");
        ASTNode *k1_var = get_named_variable_value(funcall.named_args, "K1");
        write_variable_defn(sink, len_var); fprintf(sink, ", ");
        write_variable_defn(sink, k1_var); fprintf(sink, ")");
      } else if (sdm_svncmp(funcall.name, "Bend") == 0) {
        fprintf(sink, "make_sbend(");
        ASTNode *len_var = get_named_variable_value(funcall.named_args, "L");
        ASTNode *phi_var = get_named_variable_value(funcall.named_args, "Phi");
        ASTNode *k1_var = get_named_variable_value(funcall.named_args, "K1");
        write_variable_defn(sink, len_var); fprintf(sink, ", ");
        write_variable_defn(sink, phi_var); fprintf(sink, ", ");
        write_variable_defn(sink, k1_var); fprintf(sink, ")");
      } else if (sdm_svncmp(funcall.name, "Sextupole") == 0) {
        fprintf(sink, "make_sext(");
        ASTNode *len_var = get_named_variable_value(funcall.named_args, "L");
        ASTNode *k2_var = get_named_variable_value(funcall.named_args, "K2");
        write_variable_defn(sink, len_var); fprintf(sink, ", ");
        write_variable_defn(sink, k2_var); fprintf(sink, ")");
      } else if (sdm_svncmp(funcall.name, "Octupole") == 0) {
        fprintf(sink, "make_oct(");
        ASTNode *len_var = get_named_variable_value(funcall.named_args, "L");
        ASTNode *k1_var = get_named_variable_value(funcall.named_args, "K3");
        write_variable_defn(sink, len_var); fprintf(sink, ", ");
        write_variable_defn(sink, k1_var); fprintf(sink, ")");
      } else if (sdm_svncmp(funcall.name, "Cavity") == 0) {
        fprintf(sink, "make_cavity(");
        ASTNode *freq_var = get_named_variable_value(funcall.named_args, "Frequency");
        ASTNode *volt_var = get_named_variable_value(funcall.named_args, "Voltage");
        ASTNode *harnum_var = get_named_variable_value(funcall.named_args, "HarNum");
        ASTNode *phi_var = get_named_variable_value(funcall.named_args, "Phi");
        write_variable_defn(sink, freq_var); fprintf(sink, ", ");
        write_variable_defn(sink, volt_var); fprintf(sink, ", ");
        write_variable_defn(sink, harnum_var); fprintf(sink, ", ");
        write_variable_defn(sink, phi_var); fprintf(sink, ")");
      }
    } break;
    case NODE_TYPE_COUNT: {
      fprintf(stderr, "Unreachable. This is a bug in the C transpiler.\n");
      exit(1);
    }
  }
}

void transpile_program_to_C(FILE *sink, ASTNodeArray program) {
  if (sink == stdout) printf("\n\n");

  fprintf(sink, "#include <stdio.h>\n");
  fprintf(sink, "\n");
  fprintf(sink, "#include \"acc_elements.h\"\n");
  fprintf(sink, "#include \"sdm_lib.h\"\n");
  fprintf(sink, "\n");

  fprintf(sink, "static sdm_arena_t main_arena = {0};\n");
  fprintf(sink, "static sdm_arena_t *active_arena = &main_arena;\n");
  fprintf(sink, "\n");
  fprintf(sink, "void *active_alloc(size_t size)              { return sdm_arena_alloc(active_arena, size); }\n");
  fprintf(sink, "void *active_realloc(void *ptr, size_t size) { return sdm_arena_realloc(active_arena, ptr, size); }\n");
  fprintf(sink, "\n");

  fprintf(sink, "int main(void) {\n");
  for (size_t i=0; i<program.length; i++) {
    fprintf(sink, "\t");
    write_astnode_toC(sink, &program.data[i]);
  }
  fprintf(sink, "\n");
  fprintf(sink, "\treturn 0;\n");
  fprintf(sink, "}\n");
  fprintf(sink, "\n");
}

void exit_with_error(Token *token, char *message) {
  fprintf(stderr, SDM_SV_F":%zu:%zu: %s\n", 
          SDM_SV_Vals(token->loc.filename), token->loc.line, token->loc.col, message);
  exit(1);
}

FuncSigKV *define_builtin_funcs(void) {
  FuncSigKV *builtins_dict = NULL;
  // typedef struct {
  //   sdm_string_view name;
  //   VarType return_type;
  //   VarTypeArray argument_types;
  // } FuncSig;

  char *drift_f_name = "Drift";
  VarType drift_f_intypes[] = {VAR_TYPE_FLOAT};
  FuncSig drift_f_sig = (FuncSig){.name = drift_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=drift_f_intypes, .num_args=sizeof(drift_f_intypes)/sizeof(drift_f_intypes[0])};
  shput(builtins_dict, drift_f_name, drift_f_sig);

  char *quad_f_name = "Quad";
  VarType quad_f_intypes[] = {VAR_TYPE_FLOAT, VAR_TYPE_FLOAT};
  FuncSig quad_f_sig = (FuncSig){.name = quad_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=quad_f_intypes, .num_args=sizeof(quad_f_intypes)/sizeof(quad_f_intypes[0])};
  shput(builtins_dict, quad_f_name, quad_f_sig);

  char *bend_f_name = "Bend";
  VarType bend_f_intypes[] = {VAR_TYPE_FLOAT, VAR_TYPE_FLOAT, VAR_TYPE_FLOAT};
  FuncSig bend_f_sig = (FuncSig){.name = bend_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=bend_f_intypes, .num_args=sizeof(bend_f_intypes)/sizeof(bend_f_intypes[0])};
  shput(builtins_dict, bend_f_name, bend_f_sig);

  char *sext_f_name = "Sextupole";
  VarType sext_f_intypes[] = {VAR_TYPE_FLOAT, VAR_TYPE_FLOAT};
  FuncSig sext_f_sig = (FuncSig){.name = sext_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=sext_f_intypes, .num_args=sizeof(sext_f_intypes)/sizeof(sext_f_intypes[0])};
  shput(builtins_dict, sext_f_name, sext_f_sig);

  char *oct_f_name = "Octupole";
  VarType oct_f_intypes[] = {VAR_TYPE_FLOAT, VAR_TYPE_FLOAT};
  FuncSig oct_f_sig = (FuncSig){.name = oct_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=oct_f_intypes, .num_args=sizeof(oct_f_intypes)/sizeof(oct_f_intypes[0])};
  shput(builtins_dict, oct_f_name, oct_f_sig);

  char *cavity_f_name = "Cavity";
  VarType cavity_f_intypes[] = {VAR_TYPE_FLOAT, VAR_TYPE_FLOAT};
  FuncSig cavity_f_sig = (FuncSig){.name = cavity_f_name, .return_type=VAR_TYPE_ELEMENT, .argument_types=cavity_f_intypes, .num_args=sizeof(cavity_f_intypes)/sizeof(cavity_f_intypes[0])};
  shput(builtins_dict, cavity_f_name, cavity_f_sig);

  char *println_f_name = "println";
  VarType println_f_intypes[] = {0};
  FuncSig println_f_sig = (FuncSig){.name = println_f_name, .return_type=VAR_TYPE_VOID, .argument_types=println_f_intypes, .num_args=-1};
  shput(builtins_dict, println_f_name, println_f_sig);

  char *line_f_name = "Line";
  VarType line_f_intypes[] = {0};
  FuncSig line_f_sig = (FuncSig){.name = line_f_name, .return_type=VAR_TYPE_LINE, .argument_types=line_f_intypes, .num_args=-1};
  shput(builtins_dict, line_f_name, line_f_sig);

  return builtins_dict;
}

