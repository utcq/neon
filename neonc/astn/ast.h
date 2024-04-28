#ifndef ASTN_AST_H
#define ASTN_AST_H
#include <string>
#include <vector>

enum ValType { INT };
enum OpType { ADD };
enum StatType { CALL, ASSN, RET };

struct Value {
  ValType type;
  std::string value;
};

struct Expression {
  Value *value;
  Expression *first;
  Expression *second;
  OpType *oper;
};

struct CallStat {
  std::string name;
  std::vector<Expression> arguments;
};

struct AssnStat {
  std::string dest;
  Expression *src;
};

struct RetStat {
  Expression *value;
};

struct Statement {
  StatType type;
  CallStat *callst;
  AssnStat *assnst;
  RetStat *retst;
};

struct Parameter {
  std::string name;
  std::string type;
};

struct Procedure {
    std::string name;
    std::string type;
    std::vector<Parameter> parameters;
    std::vector<Statement> statements;
};

struct Ast {
    std::vector<Procedure> procedures;
};

#endif