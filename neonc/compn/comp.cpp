#include "comp.h"
#include "../process.h"
#include "../ext/magic_enum/magic_enum.hpp"
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

Compiler::Compiler(Ast *ast_ref, std::string path, uint tmp_obj_n, Scope *scope, CompilerError *perror) {
  create_folder(".neon_tmp");
  this->emit_path = ".neon_tmp/mod_n" + std::to_string(tmp_obj_n) + ".s";
  this->writer = new Writer(this->emit_path);
  this->path = path;
  if (!scope) {
    this->scope = new Scope();
  } else {
    this->scope = scope;
  }
  if (!perror) {
    this->error = new CompilerError();
  } else {
    this->error = perror;
  }
  this->mod_id = tmp_obj_n;
  this->ext_id = this->mod_id;
  this->ast = ast_ref;
  this->scope->defs.insert(this->scope->defs.end(),
                           this->ast->ct_definitions.begin(),
                           this->ast->ct_definitions.end());
  this->emit_assembly();
  if (!perror) {
    delete this->error;
  }
}

Var *Compiler::resolve_var(std::string name) {
  for (Var *var : this->scope->vars) {
    if (var->name == name) {
      return var;
    }
  }
  this->error->VariableNotFound(name,this->path);
  return nullptr;
}

Procedure *Compiler::resolve_proc(std::string name, bool mand) {
  for (Procedure proc : this->scope->procs) {
    if (proc.name == name) {
      if (proc.setup.src_id != this->mod_id &&
          std::find(proc.setup.specifiers.begin(), proc.setup.specifiers.end(),
                    "public") == proc.setup.specifiers.end()) {
        this->error->PrivateProcedure(name, this->path);
      }
      return new Procedure(proc);
    }
  }
  if (mand) {
    this->error->ProcedureNotFound(name, this->path);
  }
  return nullptr;
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
  this->error->TypeResolveError(type, extend_def, this->path);
  return NONE;
}

uint Compiler::emit_binop(OpType oper, uint size1, uint size2) {
  if (size1 == NONE || size2 == NONE) {
    throw std::runtime_error("[UNHANDLED] Invalid operation");
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
    throw std::runtime_error("[UNHANDLED] Invalid operation");
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
        if (!var) { return NONE; }
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
    } else if (exp->value->type == ValType::REFERENCE) {
      if (exp->value->ref->value->type == ValType::OBJECT) {
        Var *defck = this->resolve_var(exp->value->ref->value->value);
        if (defck) {
          this->writer->lea(dest, new Pointer({RBP, defck->stack_offset}));
          return DWORD;
        }
      }
      else if (exp->value->ref->value->type == ValType::INT) {
        uint64_t val = std::stoull(exp->value->ref->value->value);
        this->writer->lea(dest, val);
      }
      else {
        throw std::runtime_error("Unsupported reference type");
      }
    }
    else {
      throw std::runtime_error("Invalid value");
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
  if (!stat->value){return;}
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
  if (!proc) { return NONE; } 
  for (int i = stat->arguments.size(); i > 0; i--) {
    uint size = this->emit_expression(stat->arguments[i - 1]);
    this->writer->move(regs_ord[i - 1], RAX);
  }
  this->writer->xorr(
      RAX, RAX); // TODO: This works only when XMM registers aren't being used
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
      Var *rs = this->resolve_var(val);
      if (!rs) { return; }
      coden +=
          "[rbp" + std::to_string(rs->stack_offset) + "]";
      if (i < (int)code.size()) {
        coden.push_back(code.at(i));
      }
    } else {
      coden.push_back(code.at(i));
    }
  }

  this->writer->freewrite(coden);
}

void Compiler::emit_assign(AssStat *stat) {
  Var *var = this->resolve_var(stat->name);
  if (!var) { return; }
  this->emit_expression(stat->value);
  this->writer->move(new Pointer({RBP, var->stack_offset}), RAX, var->size);
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
  case StatType::ASS: {
    this->emit_assign(stat->assst);
    break;
  }
  default: {
    break;
  }
  }
}

void Compiler::emit_procedure(Procedure *proc) {
  proc->setup.src_id = this->mod_id;
  if (this->resolve_proc(proc->name, false) != nullptr) {
    this->error->ProcedureRedefined(proc->name, this->path);
    return;
  }
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

  this->resolve_size(proc->type);

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
  Compiler *rcompiler = new Compiler(&rparser->ast, path, this->ext_id, this->scope, this->error);
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