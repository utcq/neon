#include "parser.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

Parser::Parser(Lexer *lexer_src) {
  this->tokens = lexer_src->tokens;
  this->current_token = this->tokens.at(0);
  this->size = this->tokens.size();
  while (this->index < this->size) {
    this->next();
  }
}

void Parser::advance(int by) {
  this->index += by;
  if (index + by >= this->size) {
    this->index = this->size - 1;
  }
  this->current_token = this->tokens.at(this->index);
  if (index + by >= this->size) {
    this->index = this->index + by;
  }
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

std::vector<Token *> scoped_between(TokenType type1, TokenType type2,
                                    std::vector<Token *> scope) {
  int index = 0;
  std::vector<Token *> tokens;
  if (type1 > 0) {
    assert(scope.at(index)->type == type1);
    index++;
  } else {
    tokens.push_back(scope.at(index));
    index++;
  }
  int count = 1;
  while (count > 0) {
    if (scope.at(index)->type == type2) {
      count--;
      if (count < 1) {
        break;
      }
    } else if (type1 > 0 && (scope.at(index)->type == type1)) {
      count++;
    }
    tokens.push_back(scope.at(index));
    index++;
  }
  return tokens;
}

std::vector<Token *> Parser::between(TokenType type1, TokenType type2,
                                     std::vector<Token *> scope) {
  if (scope.empty()) {
    std::vector<Token *> tokens;
    if (type1 > 0) {
      this->eat("", type1);
    } else {
      tokens.push_back(this->eat());
    }
    int count = 1;
    while (count > 0) {
      if (this->check("", type2)) {
        count--;
        if (count < 1) {
          break;
        }
      } else if (type1 > 0 && this->check("", type1)) {
        count++;
      }
      tokens.push_back(this->eat());
    }
    this->eat("", type2);
    return tokens;
  } else {
    return scoped_between(type1, type2, scope);
  }
}

void Parser::parse_proc() {
  this->eat();
  this->procsetup = ProcSetup{};
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
  this->eat("", TokenType::L_BRACE);
  std::vector<Statement> statements;
  for (int par = 1; par > 0;) {
    if (this->check("", TokenType::R_BRACE)) {
      par--;
      this->eat();
      if (par < 1) {
        break;
      }
    } else if (this->check("", TokenType::R_BRACK)) {
      par++;
      this->eat();
    }
    Statement stat = this->parse_statement();
    if (stat.type >= 0) {
      statements.push_back(stat);
    }
  }
  this->ast.procedures.push_back(
      {.name = name, .type = type, .parameters = parameters, .statements = statements, .setup=procsetup});
}

OpType parse_operator(char oper) {
  if (oper == '+') {
    return OpType::ADD;
  } else if (oper == '-') {
    return OpType::SUB;
  }
  return ADD;
}

ValType parse_type(Token *token) {
  if (token->type == TokenType::INTEGER) {
    return ValType::INT;
  } else if (token->type == TokenType::IDENTIFIER) {
    return ValType::OBJECT;
  } else if (token->type == TokenType::STRING) {
    return ValType::STR;
  }
  return ValType::OBJECT;
}

Expression *parse_exp_term(std::vector<Token *> tokens) {
  Expression *exp = new Expression();
  if (tokens.size() == 1) {
    Value *value = new Value();
    value->type = parse_type(tokens.at(0));
    value->value = tokens.at(0)->value;
    exp->value = value;
  } else {
    for (int i = 0; i < (int)tokens.size();) {
      if (tokens.at(i)->type == TokenType::L_PAREN) {
        std::vector<Token *> r_tokens = scoped_between(
            TokenType::L_PAREN, TokenType::R_PAREN,
            std::vector<Token *>(tokens.begin() + i, tokens.end()));
        i += r_tokens.size() + 2;
        if (exp->first) {
          exp->second = parse_exp_term(r_tokens);
        } else {
          exp->first = parse_exp_term(r_tokens);
        }

      } else {
        if (tokens.size() == 2) {
          assert(tokens.at(i + 1)->value.at(0) == '+' ||
                 tokens.at(i + 1)->value.at(0) == '-');
          OpType oper = parse_operator(tokens.at(i + 1)->value.at(0));
          tokens.at(i + 1)->value.erase(0, 1);
          exp->first = parse_exp_term({tokens.at(i)});
          exp->second = parse_exp_term({tokens.at(i + 1)});
          exp->oper = oper;
          i += 2;
        } else if (tokens.size() == 3) {
          OpType oper = parse_operator(tokens.at(i + 1)->value.at(0));
          exp->first = parse_exp_term({tokens.at(i)});
          exp->second = parse_exp_term({tokens.at(i + 2)});
          exp->oper = oper;
          i += 3;
        }
      }
    }

    // TODO: Implement: X + Y + Z [Polynomial]
  }

  /*if (exp->second) {
    std::cout << exp->first->value->value << " " << *(exp->oper) << " "
              << exp->second->value->value << std::endl;
  }*/
  return exp;
}

Expression *Parser::parse_expression() {
  std::vector<Token *> tokens =
      this->between((TokenType)(-1), TokenType::SEMICOLON);
  Expression *expression = parse_exp_term(tokens);
  return expression;
}

Statement Parser::parse_return() {
  RetStat *retstat = new RetStat();
  if (this->current_token->type == TokenType::SEMICOLON) {
    retstat->value = NULL;
    this->advance();
  } else {
    retstat->value = this->parse_expression();
  }
  return {.type = StatType::RET, .retst = retstat};
}

Statement Parser::parse_asmk() {
  AsmStat *asmstat = new AsmStat();
  Expression *value = this->parse_expression();
  assert (value->value && value->value->type == ValType::STR);
  asmstat->code = value->value->value;
  return {.type = StatType::ASM, .asmst=asmstat};
}

Statement Parser::parse_keyword() {
  std::string keyword = this->eat("", TokenType::IDENTIFIER)->value;
  if (keyword == "ret") {
    return this->parse_return();
  } else if (keyword == "asm") {
    return this->parse_asmk();
  }
  return {};
}

void Parser::parse_opt() {
  std::string optval = this->eat("", TokenType::IDENTIFIER)->value;
  if (optval == "pub") {
    this->procsetup.pubscope = true;
  }
}

void Parser::parse_conn() {
  std::string filepath = this->eat("", TokenType::STRING)->value;
  this->ast.connections.push_back(filepath);
}

void Parser::parse_setup() {
  std::string na = this->eat("", TokenType::IDENTIFIER)->value;
  if (na == "opt") {
    this->parse_opt();
  } else if (na == "conn") {
    this->parse_conn();
  }
}

Statement Parser::parse_statement() {
  if (this->check("%", TokenType::OPERATOR)) {
    this->eat();
    return this->parse_keyword();
  } else if(this->check("#", TokenType::HASH)) {
    this->eat();
    this->parse_setup();
  } else {
    throw std::invalid_argument("Cannot parse: " + this->current_token->value);
  }
  return {};
}

void Parser::next() {
  if (this->check("proc", TokenType::IDENTIFIER)) {
    this->parse_proc();
  } 
  else if (this->check("#", TokenType::HASH)) {
    this->eat();
    this->parse_setup();
  }
  else {
    throw std::invalid_argument("Cannot parse: " + this->current_token->value);
  }
}