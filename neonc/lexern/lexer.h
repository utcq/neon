#ifndef LEXERN_LEXER_H
#define LEXERN_LEXER_H

#include "../handler/handler.h"
#include "../comn/token.h"
#include <cstdint>
#include <sys/types.h>
#include <vector>


class Lexer {
public:
  Lexer(const char *source, std::string file);
  std::vector<Token *> tokens;

private:
  void skip_ws();
  Token *next();
  void advance(int by = 1);
  char peek(int by = 1);

  Token parse_digit();
  Token parse_xdigit();
  Token parse_identifier();
  Token parse_operator();
  Token parse_string();
  Token parse_char();
  Token parse_symbol();

  std::string source;
  uint_fast64_t offset = 0;
  uint size;
  char current_char = 1;
  Pos position;
  LexerError *error;
};

#endif