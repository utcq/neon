#ifndef ASTN_AST_H
#define ASTN_AST_H
#include <string>
#include <vector>

struct Statement;
struct Reference;
struct Dereference;

enum ValType { INT, OBJECT, STR, CH, PCALL, REFERENCE, DEREFERENCE };
enum OpType { ADD, SUB };
enum StatType { CALL, DECL, RET, ASM, ASS };

struct StateSetup {
  uint src_id = 0;
  std::vector<std::string> specifiers;
};

struct Value {
  ValType type;
  std::string value;
  Statement *stat;
  Reference *ref;
  Dereference *deref;
};

struct Reference {
  Value *value;
};

struct Dereference {
  Value *value;
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

struct DeclStat {
  std::string dest;
  std::string type;
  StateSetup setup;
  Expression *src;
};

struct RetStat {
  Expression *value;
};

struct AsmStat {
  std::string code;
};

struct AssStat {
  std::string name;
  Expression *value;
};

struct Statement {
  StatType type = (StatType)(-1);
  CallStat *callst;
  DeclStat *declst;
  RetStat *retst;
  AsmStat *asmst;
  AssStat *assst;
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
    StateSetup setup;
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