
#ifndef COMN_TOKEN_H
#define COMN_TOKEN_H


#include <string>
enum TokenType {
    INTEGER,
    IDENTIFIER,
    STRING,
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
    COMMA
};

struct Token {
    TokenType type;
    std::string value;
};

#endif