#include "lexer.h"
#include <cctype>
#include <map>
#include <set>
#include <string>
#include <vector>

std::set<std::string> operators = {"+", "-", "*", "/",  "^",  "%",  "&",
                                   "|", "<", ">", "&&", "||", "<<", ">>"};

std::map<std::string, TokenType> symbols = {
    {"=", TokenType::ASSIGN},  {";", TokenType::SEMICOLON},
    {"$", TokenType::DOLLAR},  {"(", TokenType::L_PAREN},
    {")", TokenType::R_PAREN}, {"[", TokenType::L_BRACK},
    {"]", TokenType::R_BRACK}, {",", TokenType::COMMA},
    {"{", TokenType::L_BRACE}, {"}", TokenType::R_BRACE},
    {"#", TokenType::HASH},    {".", TokenType::DOT}};

Lexer::Lexer(const char *source, std::string file) {
  this->source = (std::string(source) + " ");
  this->current_char = source[0];
  this->position = {.file = file};
  this->error = new LexerError();
  this->size = this->source.size() - 2;
  while (this->current_char && this->offset < this->size) {
    Token *tokref = this->next();
    if (tokref) {
      this->tokens.push_back(tokref);
    }
  }
  delete this->error;
}

void Lexer::advance(int by) {
  this->offset += by;
  this->position.col += by;
  this->current_char = this->source[offset];
}

Token Lexer::parse_digit() {
  std::string value;
  Pos pos = (this->position);
  while (std::isdigit(this->current_char)) {
    value.push_back(this->current_char);
    this->advance();
  }
  return {.type = TokenType::INTEGER, .value = value, .position=pos};
}

Token Lexer::parse_xdigit() {
  std::string value;
  Pos pos = (this->position);
  this->advance(2);
  while (std::isxdigit(this->current_char)) {
    value.push_back(this->current_char);
    this->advance();
  }
  value = std::to_string(std::stoll(value, 0, 16)); // hex to dec
  return {.type = TokenType::INTEGER, .value = value, .position=pos};
}

Token Lexer::parse_identifier() {
  std::string value;
  Pos pos = (this->position);
  while (std::isalnum(this->current_char) || this->current_char == '.' ||
         this->current_char == '_') {
    value.push_back(this->current_char);
    this->advance();
  }
  return {.type = TokenType::IDENTIFIER, .value = value, .position=pos};
}

Token Lexer::parse_operator() {
  std::string value;
  Pos pos = (this->position);
  value.push_back(this->current_char);
  this->advance();
  while (operators.find(value) != operators.end()) {
    value.push_back(this->current_char);
    this->advance();
  }
  value.pop_back();
  this->advance(-1);
  return {.type = TokenType::OPERATOR, .value = value, .position=pos};
}

Token Lexer::parse_string() {
  std::string value;
  Pos pos = (this->position);
  this->advance();
  while (this->current_char != '"') {
    value.push_back(this->current_char);
    this->advance();
  }
  this->advance();
  return {.type = TokenType::STRING, .value = value, .position=pos};
}

Token Lexer::parse_char() {
  std::string value;
  Pos pos = (this->position);
  this->advance();
  while (this->current_char != '\'') {
    value.push_back(this->current_char);
    this->advance();
  }
  this->advance();
  if (value == "\\n") {
    value = std::to_string('\n');
  } else if (value == "\\0") {
    value = std::to_string('\0');
  } else {
    value = std::to_string((int)value.at(0));
  }
  return {.type = TokenType::CHAR, .value = value, .position=pos};
}

Token Lexer::parse_symbol() {
  std::string value;
  Pos pos = (this->position);
  value.push_back(this->current_char);
  this->advance();
  while (symbols.find(value) != symbols.end()) {
    value.push_back(this->current_char);
    this->advance();
  }
  value.pop_back();
  this->advance(-1);
  return {.type = symbols.find(value)->second, .value = value, .position=pos};
}

char Lexer::peek(int by) { return this->source[this->offset + by]; }

void Lexer::skip_ws() {
  while (this->current_char == ' ' || this->current_char == '\n' ||
         this->current_char == '\t') {
    if (this->current_char == '\n') {
      this->position.line++;
      this->position.col = 0;
    }
    this->advance();
  }
}

Token *Lexer::next() {
  Token *resultptr = nullptr;
  if (this->offset >= this->size) {
    return resultptr;
  }

  this->skip_ws();
  if (this->current_char == 0) {
    return resultptr;
  } else if (this->current_char == '/' && this->peek() == '/') {
    while (this->current_char != '\n' && this->offset < this->size) {
      this->advance();
    }
  } else if (this->current_char == '0' && this->peek() == 'x' &&
             std::isxdigit(this->peek(2))) {
    resultptr = new Token(this->parse_xdigit());
  } else if (std::isdigit(this->current_char)) {
    resultptr = new Token(this->parse_digit());
  } else if (this->current_char == '+' || this->current_char == '-') {
    char pon = this->current_char;
    this->advance();
    this->skip_ws();
    if (this->current_char == '0' && this->peek() == 'x' &&
        std::isxdigit(this->peek(2))) {
      Token temp = this->parse_xdigit();
      temp.value = std::string(1, pon) + temp.value;
      resultptr = new Token(temp);
    } else if (std::isdigit(this->current_char)) {
      Token temp = this->parse_digit();
      temp.value = std::string(1, pon) + temp.value;
      resultptr = new Token(temp);
    } else {
      resultptr = new Token(
          {.type = TokenType::OPERATOR, .value = std::string(1, pon), .position=this->position});
    }
  }
  else if (operators.find(std::string(1, this->current_char)) !=
             operators.end()) {
    resultptr = new Token(this->parse_operator());
  }  else if (this->current_char == '\'') {
    resultptr = new Token(this->parse_char());
  } else if (this->current_char == '"') {
    resultptr = new Token(this->parse_string());
  } else if (std::isalnum(this->current_char) || this->current_char == '_') {
    resultptr = new Token(this->parse_identifier());
  } else if (symbols.find(std::string(1, this->current_char)) !=
             symbols.end()) {
    resultptr = new Token(this->parse_symbol());
  } else {
    this->error->UnknownSymbol(this->current_char, this->position);
    this->advance();
  }

  return resultptr;
}