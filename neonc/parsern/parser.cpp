#include "parser.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

Parser::Parser(Lexer *lexer_src) {
  this->tokens = lexer_src->tokens;
  this->current_token = this->tokens.at(0);
  while (this->index < lexer_src->tokens.size()) {
    this->next();
  }
}

void Parser::advance(int by) {
  this->index += by;
  this->current_token = this->tokens.at(this->index);
}

Token *Parser::eat(std::string value, TokenType type) {
  if (type >= 0) {
    if (!value.empty()) {
      assert(this->current_token->type == type &&
             this->current_token->value == value);
    } else {
      assert(this->current_token->type == type);
    }
  } else if (!value.empty()) {
    assert(this->current_token->value == value);
  }
  Token *ret = this->current_token;
  this->advance();
  return ret;
}

bool Parser::check(std::string value, TokenType type) {
  return (this->current_token->type == type &&
          (this->current_token->value == value || value.empty()));
}

std::vector<Token *> Parser::between(TokenType type1, TokenType type2) {
  std::vector<Token *> tokens;
  this->eat("", type1);
  while (!this->check("", type2)) {
    tokens.push_back(this->eat());
  }
  this->eat("", type2);
  return tokens;
}

void Parser::parse_proc() {
  this->eat();
  std::string type =
      between(TokenType::L_BRACK, TokenType::R_BRACK).at(0)->value;
  std::string name = this->eat("", TokenType::IDENTIFIER)->value;
  std::vector<Parameter> parameters;
  this->eat("", TokenType::L_PAREN);
  for (int par = 1; par > 0;) {
    if (this->check("", TokenType::R_PAREN)) {
      par--;
      this->eat();
      if (par < 1) {
        break;
      }
    } else if (this->check("", TokenType::R_PAREN)) {
      par++;
      this->eat();
    }
    this->eat("", TokenType::DOLLAR);
    std::string type = this->eat("", TokenType::IDENTIFIER)->value;
    std::string name = this->eat("", TokenType::IDENTIFIER)->value;
    if (this->check("", TokenType::COMMA)) {
      this->eat();
    }
    parameters.push_back({.name = name, .type = type});
  }
  this->eat("", TokenType::L_BRACK);
  for (int par=1; par > 0;) {
    if (this->check("", TokenType::R_BRACK)) {
      par--;
      this->eat();
      if (par < 1) {
        break;
      }
    } else if (this->check("", TokenType::R_BRACK)) {
      par++;
      this->eat();
    }
    this->parse_statement();
  }
  this->ast.procedures.push_back(
      {.name = name, .type = type, .parameters = parameters, .statements = {}});
}

Expression *Parser::parse_expression() {
  Value *value = new Value();
  value->type = ValType::INT;
  value->value = "0";
  Expression *expression = new Expression({.value=value});
  return expression;
}

Statement Parser::parse_return() {
  RetStat *retstat = new RetStat();
  retstat->value = this->parse_expression();
  return {
    .type=StatType::RET,
    .retst=retstat
  };
}

Statement Parser::parse_keyword() {
  std::string keyword = this->eat("", TokenType::IDENTIFIER)->value;
  
  if (keyword == "ret") {
    return this->parse_return();
  }
  return {};
}

Statement Parser::parse_statement() {
  if (this->check("%", TokenType::OPERATOR)) {
    this->eat();
    return this->parse_keyword();
  }
  return {};
}

void Parser::next() {
  if (this->check("proc", TokenType::IDENTIFIER)) {
    this->parse_proc();
  } else {
    throw std::invalid_argument("Cannot parse: " + this->current_token->value);
  }
}