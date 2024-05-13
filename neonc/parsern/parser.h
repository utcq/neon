#ifndef PARSERN_PARSER_H
#define PARSERN_PARSER_H
#include "../astn/ast.h"
#include "../comn/token.h"
#include "../lexern/lexer.h"
#include <cstdint>

class Parser {
public:
  Parser(Lexer *lexer_src, std::string file);
  Ast ast;

private:
  Token *eat(std::string value = "", TokenType type = (TokenType)(-1));
  bool check(std::string value, TokenType type);
  void next();
  void advance(int by = 1);
  Token *peek(int by = 1);
  std::vector<Token*> between(TokenType type1, TokenType type2, std::vector<Token*> scope={});

  void parse_proc();
  Statement parse_statement();
  Statement parse_keyword();
  void parse_setup();
  void parse_opt();
  void parse_conn();
  void parse_ctdef();
  Statement parse_call();
  Statement parse_return();
  Statement parse_asmk();
  Statement parse_declaration();
  Expression *parse_expression();

  std::string file;
  std::vector<Token *> tokens;
  uint_fast64_t index = 0;
  uint size;
  std::vector<std::string> specifiers;
  Token *current_token;
  ParserError *error;
};

#endif