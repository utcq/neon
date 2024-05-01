#include "comp.h"
#include "../process.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

bool create_folder(const std::string &path) {
  return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
}

Compiler::Compiler(Ast *ast_ref, uint tmp_obj_n, Scope *pscope) {
  create_folder(".neon_tmp");
  this->emit_path = ".neon_tmp/mod_n" + std::to_string(tmp_obj_n) + ".s";
  this->writer = new Writer(this->emit_path);
  this->ast = ast_ref;
  this->src_id = tmp_obj_n;
  if (pscope) {
    this->scope = pscope;
  } else {
    this->scope = new Scope({{},{},{},0});
  }
  this->emit_asm();
}

uint Compiler::resolve_size(std::string name) {
  if (name == "int") {
    return 4;
  } else if (name == "i64") {
    return 8;
  }
  return 8;
}

Var *Compiler::resolve_object(std::string name) {
  for (Var *local : this->scope->local_vars) {
    if (local->name == name) { return local; }
  }
  throw std::invalid_argument("Cannot find object: " + name);
}

std::string Compiler::check_func(std::string name) {
  for (Procedure procx : this->scope->procs) {
    if (procx.name == name) {
      if (procx.setup.src_id != this->src_id && !procx.setup.pubscope) {
        throw std::invalid_argument("Function: " + name + " is private");
      }
      return procx.type;
    }
  }
  throw std::invalid_argument("Cannot find function: " + name);
}


std::string Compiler::as_operation(OpType op, std::string type1, std::string type2) {
  std::string fn_name = "__primitive_" + type1 + "_" + type2;
  std::string type;
  switch (op) {
    case OpType::ADD: {
      fn_name = fn_name + "_add";
      type = this->check_func(fn_name);
      //this->writer->add("rdi", "rsi");
      break;
    } case OpType::SUB: {
      fn_name = fn_name + "_sub";
      type = this->check_func(fn_name);
      break;
    }

    default: {return type;}
  }

  this->writer->call_prologue();
  this->writer->freewrite("movl %edi, (%rax)");
  this->writer->freewrite("leaq (%rax), %rdx");
  this->writer->freewrite("pushq %rdx");

  this->writer->freewrite("movl %esi, 4(%rax)");
  this->writer->freewrite("leaq 4(%rax), %rdx");
  this->writer->freewrite("pushq %rdx");
  
  this->writer->call("nproc_" + fn_name);
  this->writer->call_epilogue();

  return type;
}

std::string Compiler::as_expression(Expression *exp) {
  if (exp->value) {
    if (exp->value->type == ValType::INT) {
      this->writer->move(new Primitive({exp->value, 8}), "rsi");
      return "int";
    } else if (exp->value->type == ValType::OBJECT) {
      Var *var = this->resolve_object(exp->value->value);
      this->writer->move(new Pointer({
        .reg = "rax",
        .offset = var->heap_offset
      }), "rsi", var->size);
      return var->type;
    }
  } else if (exp->first && ! exp->second) {
    return this->as_expression(exp->first);
  } else if (exp->first && exp->second) {
    std::string type1= this->as_expression(exp->first);
    this->writer->move("rsi", "rdi");
    std::string type2 = this->as_expression(exp->second);
    return this->as_operation(exp->oper, type1, type2);
  }
  return "";
}

void Compiler::as_return(RetStat *stat) {
  if (stat->value) {
    this->as_expression(stat->value);
    this->writer->move("rsi", "rbx");
  }
  this->writer->epilogue();
}

void Compiler::as_asmk(AsmStat *stat) {
  std::string code = stat->code;
  std::string coden = "";


  for (int i=0; i < (int)code.size(); i++) {
    if (code.at(i) == '{') {
      std::string val = "";
      i++;
      while (code.at(i) != '}') {
        val.push_back(code.at(i));
        i++;
      }
      i++;
      coden += std::to_string(this->resolve_object(val)->heap_offset) + "(%rax)";
      coden.push_back(code.at(i));
    } else {
      coden.push_back(code.at(i));
    }
  }

  this->writer->freewrite(coden);
}

void Compiler::as_statement(Statement stat) {
  switch (stat.type) {
    case StatType::RET: {
      this->as_return(stat.retst);
      break;
    }
    case StatType::ASM: {
      this->as_asmk(stat.asmst);
      break;
    }
    default: {}
  }
}

void Compiler::as_procedure(Procedure *proc) {
  this->scope->procs.push_back(*proc);

  proc->setup.src_id = this->src_id;

  if (proc->setup.pubscope) {
    this->writer->new_global("nproc_" + proc->name);
  }

  this->scope->local_vars = {};
  this->scope->proc_heap_size = 16; // Reserve Space for operations moves
  int beg = this->writer->label("nproc_" + proc->name);

  uint heap_offset=16;
  for (Parameter param : proc->parameters) {
    uint param_size = this->resolve_size(param.type);
    this->scope->proc_heap_size += param_size;
    this->writer->pop("rbx", ADDRESS);
    this->writer->move(
      new Pointer({"rbx", 0}),
      "rdi",
      param_size
    );
    this->writer->move(
      "rdi",
      new Pointer({"rax", heap_offset}),
      param_size
    );
    this->scope->local_vars.push_back(new Var({
      .name = param.name,
      .type = param.type,
      .size = param_size,
      .heap_offset = heap_offset
    }));

    heap_offset += param_size;
  }

  for (Statement stat : proc->statements) {
    this->as_statement(stat);
  }

  this->writer->prologue(beg, this->scope->proc_heap_size);
}

void Compiler::as_connect(std::string path) {
  Parser *parser = process_file(path);
  Compiler *compiler = new Compiler(&parser->ast, this->src_id+1, this->scope);
}

void Compiler::emit_asm() {
  for (std::string conn : this->ast->connections) {
    this->as_connect(conn);
  }
  for (Procedure proc : this->ast->procedures) {
    this->as_procedure(&proc);
  }
  this->writer->end();
}