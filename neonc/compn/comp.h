#ifndef COMPN_COMP_H
#define COMPN_COMP_H

#include "../astn/ast.h"
#include "../comn/token.h"
#include "common.h"
#include "writer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

struct Scope {
  std::vector<Definition> defs;
  std::vector<Var *> vars;
  std::vector<Procedure> procs;
  uint stack_size = 0;
  uint str_index = 0;
};

class Compiler {
public:
  Compiler(Ast *ast_ref, uint tmp_obj_n = 0, Scope *scope=NULL);
  std::string emit_path;
  uint ext_id=0;

  uint resolve_size(std::string type);
  Var *resolve_var(std::string name);
  std::string check_def(std::string name);
  Procedure *resolve_proc(std::string name);

  uint emit_binop(OpType oper, uint size1, uint size2);
  uint emit_expression(Expression *exp, bool first = true);

  void emit_return(RetStat *stat);
  void emit_declaration(DeclStat *stat);
  uint emit_call(CallStat *stat);
  void emit_asmkw(AsmStat *stat);

  void emit_statement(Statement *stat);

  void emit_procedure(Procedure *proc);
  void emit_assembly();

  void connect_module(std::string path);

private:
  Writer *writer;
  Ast *ast;
  Scope *scope;
  uint mod_id;
};

void small_replace(std::string &str, const std::string &from,
                   const std::string &to);

#endif