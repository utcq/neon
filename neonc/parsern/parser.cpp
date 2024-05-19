#include "parser.h"
#include "../ext/magic_enum/magic_enum.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>

std::vector<std::string> keywords = {"ret", "asm"};
std::vector<std::string> specs = {"public", "private", "extern", "intern"};

Statement parse_upcall(std::vector<Token *> btokens, ParserError *err);

Parser::Parser(Lexer *lexer_src, std::string file) {
  this->tokens = lexer_src->tokens;
  this->current_token = this->tokens.at(0);
  this->size = this->tokens.size();
  this->file = file;
  this->error = new ParserError();
  while (this->index < this->size) {
    this->next();
  }
  delete this->error;
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
      if (this->current_token->type != type ||
          this->current_token->value != value) {
        this->error->UnexpectedToken(this->current_token,
                                     magic_enum::enum_name(type).data());
      }
    } else {
      if (this->current_token->type != type) {
        this->error->UnexpectedToken(this->current_token,
                                     magic_enum::enum_name(type).data());
      }
    }
  } else if (!value.empty()) {
    if (this->current_token->value != value) {
      this->error->UnexpectedToken(this->current_token,
                                   magic_enum::enum_name(type).data());
    }
  }
  Token *ret = this->current_token;
  this->advance();
  return ret;
}

bool Parser::check(std::string value, TokenType type) {
  return (this->current_token->type == type &&
          (this->current_token->value == value || value.empty()));
}

Token *Parser::peek(int by) { return (this->tokens.at(this->index + by)); }

std::vector<Token *> scoped_between(TokenType type1, TokenType type2,
                                    std::vector<Token *> scope,
                                    ParserError *err) {
  int index = 0;
  std::vector<Token *> tokens;
  if (type1 > 0) {
    if (scope.at(0)->type != type1) {
      err->UnexpectedToken(scope.at(index),
                           magic_enum::enum_name(type1).data());
    }
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
  Pos pos = this->current_token->position;
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
      if (this->index >= this->size) {
        std::string solved = magic_enum::enum_name(type2).data();
        this->error->MissingToken(solved, pos);
        exit(1);
        return tokens;
      }
    }
    this->eat("", type2);
    return tokens;
  } else {
    return scoped_between(type1, type2, scope, this->error);
  }
}

void Parser::parse_proc() {
  this->eat();
  StateSetup setup = StateSetup{.specifiers = this->specifiers};
  this->specifiers.clear();

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
    if (this->check("", TokenType::DOT) &&
        this->peek()->type == TokenType::DOT &&
        this->peek(2)->type == TokenType::DOT) {
      // Variadic
      this->eat();
      this->eat();
      this->eat();
      parameters.push_back({.name = "...", .type = "variadic"});
    } else {
      this->eat("", TokenType::DOLLAR);
      std::string type = this->eat("", TokenType::IDENTIFIER)->value;
      std::string name = this->eat("", TokenType::IDENTIFIER)->value;
      parameters.push_back({.name = name, .type = type});
    }
    if (this->check("", TokenType::COMMA)) {
      this->eat();
    }
  }
  std::vector<Statement> statements;
  if (!this->check(";", TokenType::SEMICOLON)) {
    if (this->eat("", TokenType::L_BRACE)->type != TokenType::L_BRACE) {
      while (!this->check("proc", TokenType::IDENTIFIER) &&
             this->index < this->tokens.size()) {
        this->eat();
      }
    } else {
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
    }
  } else {
    this->eat();
  }
  this->ast.procedures.push_back({.name = name,
                                  .type = type,
                                  .parameters = parameters,
                                  .statements = statements,
                                  .setup = setup});
}

OpType parse_operator(std::string oper) {
  if (oper == "+") {
    return OpType::ADD;
  } else if (oper == "-") {
    return OpType::SUB;
  }
  return (OpType)(-1);
}

ValType parse_type(Token *token) {
  if (token->type == TokenType::INTEGER) {
    return ValType::INT;
  } else if (token->type == TokenType::IDENTIFIER) {
    return ValType::OBJECT;
  } else if (token->type == TokenType::STRING) {
    return ValType::STR;
  } else if (token->type == TokenType::CHAR) {
    return ValType::CH;
  }
  return ValType::OBJECT;
}

Expression *parse_exp_term(std::vector<Token *> tokens, ParserError *err) {
  Expression *exp = new Expression();
  if (tokens.size() >= 2 && tokens.at(0)->type == TokenType::IDENTIFIER &&
      tokens.at(1)->type == TokenType::L_PAREN) {
    Value *value = new Value();
    value->type = ValType::PCALL;
    value->stat = new Statement(parse_upcall(tokens, err));
    value->value = "x";
    exp->value = value;
  } else if (tokens.size() == 1) {
    Value *value = new Value();
    value->type = parse_type(tokens.at(0));
    value->value = tokens.at(0)->value;
    exp->value = value;
  } else {
    for (int i = 0; i < (int)tokens.size();) {
      if (tokens.at(i)->type == TokenType::L_PAREN) {
        std::vector<Token *> r_tokens = scoped_between(
            TokenType::L_PAREN, TokenType::R_PAREN,
            std::vector<Token *>(tokens.begin() + i, tokens.end()), err);
        i += r_tokens.size() + 2;
        if (exp->first) {
          exp->second = parse_exp_term(r_tokens, err);
        } else {
          exp->first = parse_exp_term(r_tokens, err);
        }
      } else {
        if (tokens.size() == 2) {
          if (tokens.at(i)->type == TokenType::OPERATOR &&
              tokens.at(i)->value.at(0) == '&') {
            Value *value = new Value();
            value->type = ValType::REFERENCE;
            Expression *xp = parse_exp_term({tokens.at(i + 1)}, err);
            value->ref = new Reference({.value =
              xp->value
            });
            exp->value = value;
          } else if (tokens.at(i)->type == TokenType::OPERATOR &&
                     tokens.at(i)->value.at(0) == '*') {
            Value *value = new Value();
            value->type = ValType::DEREFERENCE;
            value->deref = new Dereference({.value =
              parse_exp_term({tokens.at(i + 1)}, err)->first->value
            });
            exp->value = value;
          } else {
            assert(tokens.at(i + 1)->value.at(0) == '+' ||
                   tokens.at(i + 1)->value.at(0) == '-');
            OpType oper =
                parse_operator(std::string(1, tokens.at(i + 1)->value.at(0)));
            if (oper < 0) {
              err->UnknownOperator(tokens.at(i + 1));
            }
            tokens.at(i + 1)->value.erase(0, 1);
            exp->first = parse_exp_term({tokens.at(i)}, err);
            exp->second = parse_exp_term({tokens.at(i + 1)}, err);
            exp->oper = oper;
          }
          i += 2;
        } else if (tokens.size() == 3) {
            OpType oper = parse_operator(tokens.at(i + 1)->value);
            if (oper < 0) {
              err->UnknownOperator(tokens.at(i + 1));
            }
            exp->first = parse_exp_term({tokens.at(i)}, err);
            exp->second = parse_exp_term({tokens.at(i + 2)}, err);
            exp->oper = oper;
            i += 3;
        } else {
          if (tokens.at(i)->type == TokenType::OPERATOR &&
              tokens.at(i)->value == "&") {
            Value *value = new Value();
            value->type = ValType::REFERENCE;
            value->value = tokens.at(i + 1)->value;
            exp->value = value;
            i += 2;
          } else if (tokens.at(i)->type == TokenType::OPERATOR &&
                     tokens.at(i)->value == "*") {
            Value *value = new Value();
            value->type = ValType::DEREFERENCE;
            value->value = tokens.at(i + 1)->value;
            exp->value = value;
            i += 2;
          }
        }
      }
    }

    // TODO: Implement: X + Y + Z [Polynomial]
  }
  return exp;
}

Expression *Parser::parse_expression() {
  std::vector<Token *> tokens =
      this->between((TokenType)(-1), TokenType::SEMICOLON);
  Expression *expression = parse_exp_term(tokens, this->error);
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
  std::unique_ptr<AsmStat> asmstat(new AsmStat());
  Token *tok = this->current_token;
  Expression *value = this->parse_expression();
  std::string val = value->value->value;
  if (value && value->value && value->value->type != ValType::STR) {
    this->error->UnexpectedToken(tok, "STRING");
  }
  asmstat->code = val;

  return {.type = StatType::ASM, .asmst = asmstat.release()};
}

Statement Parser::parse_keyword() {
  std::string keyword = this->eat("", TokenType::IDENTIFIER)->value;
  if (keyword == "ret") {
    return this->parse_return();
  } else if (keyword == "asm") {
    Statement asmpk = this->parse_asmk();
    return asmpk;
  }
  return {};
}

void Parser::parse_opt() {
  std::string optval = this->eat("", TokenType::IDENTIFIER)->value;
  // if (optval == "pub") {
  //   this->statesetup.pubscope = true;
  // }
}

void Parser::parse_conn() {
  std::string filepath = this->eat("", TokenType::STRING)->value;
  this->ast.connections.push_back(filepath);
}

void Parser::parse_ctdef() {
  std::string name = this->eat("", TokenType::IDENTIFIER)->value;
  std::string val = this->eat("")->value;
  this->ast.ct_definitions.push_back({.name = name, .value = val});
}

void Parser::parse_setup() {
  std::string na = this->eat("", TokenType::IDENTIFIER)->value;
  if (na == "opt") {
    this->parse_opt();
  } else if (na == "conn") {
    this->parse_conn();
  } else if (na == "define") {
    this->parse_ctdef();
  }
}

Statement parse_upcall(std::vector<Token *> btokens, ParserError *err) {
  std::vector<Expression *> args;
  std::string name = btokens.at(0)->value;
  btokens.erase(btokens.begin());
  std::vector<Token *> tokens =
      scoped_between(TokenType::L_PAREN, TokenType::R_PAREN, btokens, err);
  std::vector<Token *> ctoks;
  for (Token *tok : tokens) {
    if (tok->type == TokenType::COMMA) {
      Expression *exp = parse_exp_term(ctoks, err);
      args.push_back(exp);
      ctoks = {};
    } else {
      ctoks.push_back(tok);
    }
  }
  if (!ctoks.empty()) {
    args.push_back(parse_exp_term(ctoks, err));
  }
  return {.type = StatType::CALL,
          .callst = new CallStat({.name = name, .arguments = args})};
}

Statement Parser::parse_call() {
  return parse_upcall(this->between((TokenType)(-1), TokenType::SEMICOLON),
                      this->error);
}

Statement Parser::parse_declaration() {
  StateSetup setup = {.specifiers = this->specifiers};
  this->specifiers.clear();
  this->eat();
  this->eat("$", TokenType::DOLLAR);
  std::string type = this->eat("", TokenType::IDENTIFIER)->value;
  std::string name = this->eat("", TokenType::IDENTIFIER)->value;
  this->eat("=", TokenType::ASSIGN);
  Expression *express = this->parse_expression();
  return {.type = StatType::DECL,
          .declst = new DeclStat(
              {.dest = name, .type = type, .setup = setup, .src = express})};
}

Statement Parser::parse_assign() {
  std::string name = this->eat("", TokenType::IDENTIFIER)->value;
  this->eat("=", TokenType::ASSIGN);
  Expression *exp = this->parse_expression();
  return {.type = StatType::ASS,
          .assst = new AssStat({.name = name, .value = exp})};
}

Statement Parser::parse_statement() {
  if (this->check("", TokenType::IDENTIFIER) &&
      (std::find(keywords.begin(), keywords.end(),
                 this->current_token->value) != keywords.end())) {
    return this->parse_keyword();
  } else if (this->check("#", TokenType::HASH)) {
    this->eat();
    this->parse_setup();
  } else if (this->check("", TokenType::IDENTIFIER) &&
             this->peek()->type == TokenType::L_PAREN) {
    return this->parse_call();
  } else if (this->check("let", TokenType::IDENTIFIER) &&
             this->peek()->type == TokenType::DOLLAR &&
             this->peek(2)->type == TokenType::IDENTIFIER) {
    return this->parse_declaration();
  } else if (this->check("", TokenType::IDENTIFIER) &&
             this->peek()->type == TokenType::ASSIGN) {
    return this->parse_assign();
  } else {
    this->error->UnknownStatement(this->current_token);
    uint line = this->current_token->position.line;
    while (this->current_token->position.line == line &&
           this->index < this->tokens.size()) {
      this->advance();
    }
  }
  return {};
}

void Parser::next() {
  if (this->check("", TokenType::IDENTIFIER) &&
      (std::find(specs.begin(), specs.end(), this->current_token->value) !=
       specs.end())) {
    this->specifiers.push_back(this->eat()->value);
  } else if (this->check("proc", TokenType::IDENTIFIER)) {
    this->parse_proc();
  } else if (this->check("#", TokenType::HASH)) {
    this->eat();
    this->parse_setup();
  } else {
    this->error->UnknownWhat(this->current_token);
    uint line = this->current_token->position.line;
    while (this->current_token->position.line == line &&
           this->index < this->tokens.size()) {
      this->advance();
    }
  }
}
