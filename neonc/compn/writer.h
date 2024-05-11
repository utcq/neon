#ifndef COMPN_WRITER_H
#define COMPN_WRITER_H

#include "../astn/ast.h"
#include "common.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <cassert>


#define RAX "rax"
#define RBX "rbx"
#define RCX "rcx"
#define RDX "rdx"
#define RSI "rsi"
#define RSP "rsp"
#define RBP "rbp"

struct Translation {
  std::string mnemonic;
  std::string reg_sz;
};

class Writer {
public:
  Writer(std::string file);

  // MNG
  void label(std::string name);
  void new_global(std::string name);
  void new_string(std::string name, std::string value);

  // Instructions
  void push(std::string src_register);
  void move(std::string dst_register, std::string src_register);
  void move(std::string dst_register, int64_t primitive);
  void move(Pointer *ptr, std::string src_register, uint size);
  void move(std::string dst_register, Pointer *src_ptr, uint size);
  void move(std::string dst_register, std::string src_label, uint size);

  void ret();

  void pop(std::string dst_register);

  void call(std::string name);

  // Utils
  void freewrite(std::string code);
  void function_prologue();
  void function_epilogue();

  void replace_free(std::string from, std::string to);

  void end();
private:
  int get_txt_pos();

  std::ofstream asm_out;
  std::stringstream asm_global;
  std::stringstream asm_text;
  std::stringstream asm_data;
  std::stringstream asm_rodata;
};

#endif