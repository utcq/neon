#ifndef ASTN_AST_H
#define ASTN_AST_H
#include <string>
#include <vector>

struct Statement;

enum ValType { INT, OBJECT, STR, PCALL };
enum OpType { ADD, SUB };
enum StatType { CALL, ASSN, RET, ASM };

struct Value {
  ValType type;
  std::string value;
  Statement *stat;
};

struct Expression {
  Value *value;
  Expression *first;
  Expression *second;
  OpType oper;
};

struct CallStat {
  std::string name;
  std::vector<Expression*> arguments;
};

struct AssnStat {
  std::string dest;
  Expression *src;
};

struct RetStat {
  Expression *value;
};

struct AsmStat {
  std::string code;
};

struct Statement {
  StatType type = (StatType)(-1);
  CallStat *callst;
  AssnStat *assnst;
  RetStat *retst;
  AsmStat *asmst;
};

struct Parameter {
  std::string name;
  std::string type;
};

struct ProcSetup {
  uint src_id = 0;
  bool pubscope=false;
};

struct Procedure {
    std::string name;
    std::string type;
    std::vector<Parameter> parameters;
    std::vector<Statement> statements;
    ProcSetup setup;
};

struct Definition {
  std::string name;
  std::string value;
};

struct Ast {
    std::vector<Procedure> procedures;
    std::vector<std::string> connections;
    std::vector<Definition> ct_definitions;
};

#endif