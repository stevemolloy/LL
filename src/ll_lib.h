#ifndef _LL_LIB_H
#define _LL_LIB_H

#include <stdlib.h>
#include <stdio.h>

#include "sdm_lib.h"

#ifndef EXTERN
#define EXTERN extern
#endif

typedef enum {
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_VARINIT,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_ASSIGNMENT,
  TOKEN_TYPE_ADD,
  TOKEN_TYPE_MULT,
  TOKEN_TYPE_SUB,
  TOKEN_TYPE_DIV,
  TOKEN_TYPE_OPAREN,
  TOKEN_TYPE_CPAREN,
  TOKEN_TYPE_SEMICOLON,
  TOKEN_TYPE_COLON,
  TOKEN_TYPE_COMMA,
  TOKEN_TYPE_COMMENT,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_COUNT,
} TokenType;

typedef struct {
  sdm_string_view filename;
  size_t line;
  size_t col;
} Loc;

typedef struct {
  TokenType type;
  sdm_string_view content;
  Loc loc;
} Token;

typedef struct {
  size_t capacity;
  size_t length;
  Token *data;
  size_t index;
} TokenArray;

typedef struct {
  sdm_string_view filename;
  sdm_string_view contents;
} FileData;

typedef struct ASTNode ASTNode;

typedef enum {
  NODE_TYPE_BINOP,
  NODE_TYPE_LITERAL,
  NODE_TYPE_VARINIT,
  NODE_TYPE_VARIABLE,
  NODE_TYPE_FUNCALL,
  NODE_TYPE_COUNT,
} ASTNodeType;

typedef enum {
  VAR_TYPE_INT,
  VAR_TYPE_FLOAT,
  VAR_TYPE_STRING,
  VAR_TYPE_ELEDRIFT,
  VAR_TYPE_ELEQUAD,
  VAR_TYPE_ELEMARKER,
  VAR_TYPE_ELEBEND,
  VAR_TYPE_ELESEXTUPOLE,
  VAR_TYPE_ELEOCTUPOLE,
  VAR_TYPE_ELECAVITY,
  VAR_TYPE_COUNT,
} VarType;

typedef enum {
  BINOP_ADD,
  BINOP_SUB,
  BINOP_MUL,
  BINOP_DIV,
  BINOP_COUNT,
} BinOpType;

typedef struct {
  BinOpType type;
  VarType result_type;
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

typedef struct {
  size_t capacity;
  size_t length;
  VariableInit *data;
} VariableInitArray;

typedef struct {
  sdm_string_view name;
} Variable;

typedef struct ASTNodeArray ASTNodeArray;
typedef struct {
  sdm_string_view name;
  ASTNodeArray *args;
  VariableInitArray *named_args;
} FunCall;

struct ASTNode {
  ASTNodeType type;
  union {
    BinOp binop;
    Literal literal;
    VariableInit var_init;
    Variable variable;
    FunCall funcall;
  } as;
};

struct ASTNodeArray {
  size_t capacity;
  size_t length;
  struct ASTNode *data;
};

typedef struct {
  char *key;
  ASTNode *value;
} VariableDefn;

EXTERN VariableDefn *variable_lib;

Token *get_next_token(TokenArray *token_array);
void expect_next_token_type(TokenArray *token_array, TokenType type, char *err_mesg);
void validate_token_array(TokenArray *token_array);
bool tokenise_input_file(FileData *file_data, TokenArray *token_array);
size_t starts_with_float(const char *input);
bool isvalididchar(char c);
bool string_starts_with(const char *input, const char *needle);
void sv_trim_follow(sdm_string_view *SV, Loc *loc);
sdm_string_view sv_chop_follow(sdm_string_view *SV, size_t len, Loc *loc);

Token *get_current_token(TokenArray *token_array);
#define print_ast(ARGS) print_ast_((ARGS), 0)
void print_ast_(ASTNode *ast, size_t level);
ASTNode *parse_expression_primary(TokenArray *token_array);
ASTNode *parse_expression_multdiv(TokenArray *token_array);
ASTNode *parse_expression_plus_minus(TokenArray *token_array);
ASTNode *parse_variable_initiation(TokenArray *token_array);
ASTNode *parse_expression(TokenArray *token_array);

void write_astnode_toC(FILE *sink, ASTNode *ast);
void transpile_program_to_C(FILE *sink, ASTNodeArray program);

void exit_with_error(Token *token, char *message);

#endif //_LL_LIB_H

