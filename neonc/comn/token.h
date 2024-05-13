#include <cstdint>
#ifndef COMN_TOKEN_H
#define COMN_TOKEN_H


#include <string>


struct Pos {
  uint_fast32_t line=1;
  uint_fast32_t col=1;
  std::string file;
};

enum TokenType {
    INTEGER,
    IDENTIFIER,
    STRING,
    CHAR,
    OPERATOR,
    ASSIGN,
    SEMICOLON,
    DOLLAR,
    L_PAREN,
    R_PAREN,
    L_BRACK,
    R_BRACK,
    L_BRACE,
    R_BRACE,
    COMMA,
    HASH,
    DOT
};

struct Token {
    TokenType type;
    std::string value;
    Pos position;
};

#endif