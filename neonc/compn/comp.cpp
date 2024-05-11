#include "comp.h"
#include "../process.h"
#include "common.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

std::string regs_ord[12] = {"rdi", "rsi", "rdx", "rcx", "r8",  "r9",
                            "r10", "r11", "r12", "r13", "r14", "r15"};

bool create_folder(const std::string &path) {
  return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

Compiler::Compiler(Ast *ast_ref, uint tmp_obj_n, Scope *scope) {
  create_folder(".neon_tmp");
  this->emit_path = ".neon_tmp/mod_n" + std::to_string(tmp_obj_n) + ".s";
  this->writer = new Writer(this->emit_path);
  if (!scope) {
    this->scope = new Scope();
  } else {
    this->scope = scope;
  }
  this->mod_id = tmp_obj_n;
  this->ext_id = this->mod_id;
  this->ast = ast_ref;
  this->scope->defs.insert(this->scope->defs.end(),
                           this->ast->ct_definitions.begin(),
                           this->ast->ct_definitions.end());
  this->emit_assembly();
}

Var *Compiler::resolve_var(std::string name) {
  for (Var *var : this->scope->vars) {
    if (var->name == name) {
      return var;
    }
  }
  throw std::runtime_error("Cannot find variable: " + name);
}

Procedure *Compiler::resolve_proc(std::string name) {
  for (Procedure proc : this->scope->procs) {
    if (proc.name == name) {
      if (proc.setup.src_id != this->mod_id &&
          std::find(proc.setup.specifiers.begin(), proc.setup.specifiers.end(),
                    "public") == proc.setup.specifiers.end()) {
                      throw std::runtime_error("Cannot access private procedure: " + name);
      }
      return new Procedure(proc);
    }
  }
  throw std::runtime_error("Cannot find procedure: " + name);
}

uint Compiler::resolve_size(std::string type) {
  std::string extend_def = ("__size_" + type);
  for (uint i = 0; i < extend_def.size(); i++) {
    extend_def[i] = std::toupper(extend_def.at(i));
  }
  for (Definition def : this->scope->defs) {
    if (def.name == extend_def) {
      return std::stoul(def.value);
    }
  }
  throw std::runtime_error("Cannot find type size: [definition: " + extend_def +
                           "]");
}

uint Compiler::emit_binop(OpType oper, uint size1, uint size2) {
  if (size1 == NONE || size2 == NONE) {
    throw std::runtime_error("Invalid operation");
  }
  switch (oper) {
  case OpType::ADD: {
    this->writer->freewrite("add rax,rbx");
    break;
  }
  case OpType::SUB: {
    this->writer->freewrite("sub rax,rbx");
    break;
  }
  default: {
    throw std::runtime_error("Invalid operation");
    break;
  }
  }

  if (size1 > size2) {
    return size1;
  }
  return size2;
}

std::string Compiler::check_def(std::string value) {
  for (Definition def : this->scope->defs) {
    if (def.name == value) {
      return def.value;
    }
  }
  return "";
}

uint Compiler::emit_expression(Expression *exp, bool first) {
  if (exp->value) {
    std::string dest = (first) ? RAX : RBX;
    if (exp->value->type == ValType::INT) {
      int64_t val = std::stoll(exp->value->value);
      this->writer->move(dest, val);
      if (val > INT32_MAX) {
        return QWORD;
      }
      return DWORD;
    } else if (exp->value->type == ValType::CH) {
      char val = std::stoul(exp->value->value);
      this->writer->move(dest, val);
      return BYTE;
    } else if (exp->value->type == ValType::OBJECT) {
      std::string defck = this->check_def(exp->value->value);
      if (!defck.empty()) {
        this->writer->move(dest, std::stoi(defck));
        return DWORD;
      } else {
        Var *var = this->resolve_var(exp->value->value);
        this->writer->move(dest, new Pointer({RBP, var->stack_offset}),
                           var->size);
        return var->size;
      }
    } else if (exp->value->type == ValType::PCALL) {
      return this->emit_call(exp->value->stat->callst);
    } else if (exp->value->type == ValType::STR) {
      std::string strname =
          "CONST_STR_" + std::to_string(this->scope->str_index);
      this->writer->new_string(strname, exp->value->value);
      this->writer->move(dest, strname, DWORD);
      this->scope->str_index++;
      return DWORD;
    }
  } else if (exp->first && !exp->second) {
    this->emit_expression(exp->first, first);
  } else {
    uint size1 = this->emit_expression(exp->first, first);
    uint size2 = this->emit_expression(exp->second, false);
    return this->emit_binop(exp->oper, size1, size2);
  }
  return NONE;
}

void Compiler::emit_return(RetStat *stat) {
  this->emit_expression(stat->value);
}

void Compiler::emit_declaration(DeclStat *stat) {
  uint size = this->resolve_size(stat->type);
  int stack_offset = 0;
  if (!this->scope->vars.empty()) {
    stack_offset = this->scope->vars.back()->stack_offset;
  }
  stack_offset -= size;
  this->scope->vars.push_back(
      new Var({stat->dest, stat->type, size, stack_offset}));
  this->scope->stack_size += size;
  this->emit_expression(stat->src);
  this->writer->move(new Pointer({RBP, stack_offset}), RAX, size);
}

uint Compiler::emit_call(CallStat *stat) {
  Procedure *proc = this->resolve_proc(stat->name);
  for (int i = stat->arguments.size(); i > 0; i--) {
    uint size = this->emit_expression(stat->arguments[i - 1]);
    this->writer->move(regs_ord[i - 1], RAX);
  }
  this->writer->call(stat->name);
  return this->resolve_size(proc->type);
}

void Compiler::emit_asmkw(AsmStat *stat) {
  std::string code = stat->code;
  std::string coden = "";

  for (int i = 0; i < (int)code.size(); i++) {
    if (code.at(i) == '{') {
      std::string val = "";
      i++;
      while (code.at(i) != '}') {
        val.push_back(code.at(i));
        i++;
      }
      i++;
      coden +=
          "[rbp" + std::to_string(this->resolve_var(val)->stack_offset) + "]";
      if (i < (int)code.size()) {
        coden.push_back(code.at(i));
      }
    } else {
      coden.push_back(code.at(i));
    }
  }

  this->writer->freewrite(coden);
}

void Compiler::emit_statement(Statement *stat) {
  switch (stat->type) {
  case StatType::RET: {
    this->emit_return(stat->retst);
    break;
  }
  case StatType::ASM: {
    this->emit_asmkw(stat->asmst);
    break;
  }
  case StatType::DECL: {
    this->emit_declaration(stat->declst);
    break;
  }
  case StatType::CALL: {
    this->emit_call(stat->callst);
    break;
  }
  default: {
    break;
  }
  }
}

void Compiler::emit_procedure(Procedure *proc) {
  proc->setup.src_id = this->mod_id;
  this->scope->procs.push_back(*proc);
  this->scope->vars.clear();
  this->scope->stack_size = 0;

  for (std::string spec : proc->setup.specifiers) {
    if (spec == "public") {
      this->writer->new_global(proc->name);
    } else if (spec == "extern") {
      return;
    }
  }

  this->writer->label(proc->name);
  this->writer->function_prologue();

  int stack_offset = 0;
  uint param_size = 0;
  for (uint i = 0; i < proc->parameters.size(); i++) {
    param_size = this->resolve_size(proc->parameters[i].type);
    stack_offset -= param_size;
    this->writer->move(new Pointer({RBP, stack_offset}), regs_ord[i],
                       param_size);
    this->scope->vars.push_back(
        new Var({proc->parameters[i].name, proc->parameters[i].type, param_size,
                 stack_offset}));
    this->scope->stack_size += param_size;
  }

  for (Statement stat : proc->statements) {
    this->emit_statement(&stat);
  }
  this->writer->replace_free("@STACK_SIZE",
                             std::to_string(this->scope->stack_size));
  this->writer->function_epilogue();
  this->writer->ret();
}

void Compiler::connect_module(std::string path) {
  Parser *rparser = process_file(path);
  this->ext_id++;
  Compiler *rcompiler = new Compiler(&rparser->ast, this->ext_id, this->scope);
  this->ext_id = rcompiler->ext_id;
}

void Compiler::emit_assembly() {
  for (std::string path : this->ast->connections) {
    this->connect_module(path);
  }

  for (Procedure proc : this->ast->procedures) {
    this->emit_procedure(&proc);
  }

  this->writer->end();
}