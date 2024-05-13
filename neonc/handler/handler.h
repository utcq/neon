#include "../comn/token.h"
#ifndef HND_HANDLER_H
#define HND_HANDLER_H
#include <iostream>
#include <string>

class LexerError {
public:
  LexerError();
  ~LexerError();

  void UnknownSymbol(char symbol, Pos pos);
private:
    bool err_flag;
};

class ParserError {
public:
  ParserError();
  ~ParserError();

  void UnknownStatement(Token *token);
  void UnknownOperator(Token *token);
  void UnknownWhat(Token *token);
  void MissingToken(std::string token, Pos pos);
  void UnexpectedToken(Token *token, std::string expected);
private:
    bool err_flag;
};

class CompilerError {
public:
  CompilerError();
  ~CompilerError();

  void VariableNotFound(std::string name, std::string file);
  void PrivateProcedure(std::string name, std::string file);
  void ProcedureNotFound(std::string name, std::string file);
  void TypeResolveError(std::string name, std::string ext, std::string file);
  void ProcedureRedefined(std::string name, std::string file);
private:
    bool err_flag;
};

class FileError {
public:
  FileError();
  ~FileError();

  void MissingFile(std::string file);
private:
    bool err_flag;
};

#endif