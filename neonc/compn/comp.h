#ifndef COMPN_COMP_H
#define COMPN_COMP_H

#include "../astn/ast.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define ADDRESS 8

struct Translation {
  std::string reg;
  std::string mnm;
};

struct Var {
  std::string name;
  std::string type;
  uint size;
  uint heap_offset;
};

struct Pointer {
  std::string reg;
  uint offset;
};

struct Primitive {
  Value *val;
  uint size;
};

struct Scope {
  std::vector<Procedure> procs;
  std::vector<Var*> local_vars;
  std::vector<Var*> global_vars;
  std::vector<Definition> ct_definitions;
  uint proc_heap_size;
};

class Writer {
public:
  Writer(std::string file);

  void prologue(int pos, uint heap_size);
  void epilogue();

  int label(std::string name);

  void move(Primitive *val, std::string reg);
  void move(Pointer *pointer, std::string reg, uint size);
  void move(std::string reg_src, std::string reg_dst);
  void move(std::string reg, Pointer *pointer, uint size);

  void pop(std::string reg, uint size);

  void add(std::string reg_src, std::string reg_dst);

  void call_prologue();
  void call(std::string name);
  void call_epilogue();

  void lea(Pointer *val, std::string reg);
  void push(std::string reg);

  void new_global(std::string name);

  void freewrite(std::string code);

  void end();
private:
  int get_txt_pos();

  std::ofstream asm_out;
  std::stringstream asm_global;
  std::stringstream asm_text;
  std::stringstream asm_data;
};

class Compiler {
public:
  Compiler(Ast *ast_ref, uint tmp_obj_n = 0, Scope *pscope = NULL);
  std::string emit_path;

private:
  uint resolve_size(std::string name);
  void emit_asm();
  void as_procedure(Procedure *proc);
  void as_statement(Statement stat);
  void as_return(RetStat *stat);
  void as_asmk(AsmStat *stat);
  void as_call(CallStat *stat);
  std::string as_expression(Expression *exp);
  std::string as_operation(OpType op, std::string type1, std::string type2);
  Procedure check_func(std::string name);

  void as_connect(std::string path);

  Var *resolve_object(std::string name);
  
  uint src_id=0;
  Writer *writer;
  Scope *scope;
  Ast *ast;
};

void small_replace(std::string& str, const std::string& from, const std::string& to);

#endif