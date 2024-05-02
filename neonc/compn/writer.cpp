#include "comp.h"
#include <cassert>
#include <fstream>
#include <map>

std::map<std::string, std::vector<std::string>> registers_map = {
    {"rax", {"ah", "ax", "eax", "rax"}},  {"rbx", {"bh", "bx", "ebx", "rbx"}},
    {"rcx", {"ch", "cx", "ecx", "rcx"}},  {"rdi", {"dil", "di", "edi", "rdi"}},
    {"rsi", {"sil", "si", "esi", "rsi"}}, {"rdx", {"dh", "dx", "edx", "rdx"}}};

void Writer::end() {
  this->asm_out << this->asm_global.str();
  this->asm_out << this->asm_data.str() << "\n\n";
  this->asm_out << this->asm_text.str() << "\n";
  this->asm_out.close();
}

Translation translate_mn(std::string mnm, std::string reg, uint size) {
  assert(size <= 8);
  std::vector<std::string> reg_map = registers_map.find(reg)->second;
  if (size == 1) {
    return {.reg = reg_map[0], .mnm = mnm + "b"};
  } else if (size == 2) {
    return {.reg = reg_map[1], .mnm = mnm + "w"};
  } else if (size == 4) {
    return {.reg = reg_map[2], .mnm = mnm + "l"};
  } else if (size == 8) {
    return {.reg = reg_map[3], .mnm = mnm + "q"};
  } else {
    assert(0 == 1);
  }
}

void Writer::call_prologue() {
  this->asm_text << "\tpushq %rax\n"
                 << "\tpushq %rbp\n";
}

void Writer::call(std::string name) {
  this->asm_text << "\tcall " << name << "\n";
}

void Writer::call_epilogue() {
  this->asm_text << "\tpopq %rbp\n"
                 << "\tpopq %rax\n";
}

void Writer::move(Primitive *val, std::string reg) {
  if (val->val->type == ValType::INT) {
    Translation trns = translate_mn("mov", reg, val->size);
    this->asm_text << "\t" << trns.mnm << " $" << val->val->value << ", %"
                   << trns.reg << "\n";
  }
}

void Writer::move(std::string reg_src, std::string reg_dst) {
  this->asm_text << "\tmovq %" << reg_src << ", %" << reg_dst << '\n';
}

void Writer::move(Pointer *pointer, std::string reg, uint size) {
  Translation trns = translate_mn("mov", reg, size);
  this->asm_text << "\t" << trns.mnm << " " << (pointer->offset) << "(%"
                 << pointer->reg << "), %" << trns.reg << "\n";
}

void Writer::move(std::string reg, Pointer *pointer, uint size) {
  Translation trns = translate_mn("mov", reg, size);
  this->asm_text << "\t" << trns.mnm << " %" << trns.reg << ", "
                 << pointer->offset << "(%" << pointer->reg << ")\n";
}

void Writer::pop(std::string reg, uint size) {
  Translation trns = translate_mn("pop", reg, size);
  this->asm_text << "\t" << trns.mnm << " %" << trns.reg << "\n";
}

void Writer::add(std::string reg_src, std::string reg_dst) {
  this->asm_text << "\taddq %" << reg_src << ", %" << reg_dst << "\n";
}

void Writer::lea(Pointer *pointer, std::string reg) {
  this->asm_text << "\tleaq " << (pointer->offset) << "(%" << pointer->reg
                 << "), %" << reg << "\n";
}

void Writer::push(std::string reg) {
  this->asm_text << "\tpushq %" << reg << "\n";
}

int Writer::label(std::string name) {
  this->asm_text << name << ":\n";
  int beg_pos = this->get_txt_pos();
  this->asm_text << std::string(44, ' '); // Space for Prologue
  return beg_pos;
}

int Writer::get_txt_pos() { return (int)(this->asm_text.tellp()); }

void Writer::prologue(int pos, uint heap_size) {
  int cur = this->get_txt_pos();
  std::string heap_val =
      std::string(6 - std::to_string(heap_size).length(), '0') +
      std::to_string(heap_size);
  this->asm_text.seekp(pos);
  this->asm_text << "\tpopq %rbp\n"
                 << "\tmovq $" << heap_val << ", %rsi\n"
                 << "\tcall halloc\n";
  this->asm_text.seekp(cur);
}

void Writer::epilogue() {
  this->asm_text << "\tpushq %rbp\n"
                 << "\tret\n\n\n";
}

void small_replace(std::string &str, const std::string &from,
                   const std::string &to) {
  size_t start = 0;
  while ((start = str.find(from, start)) != std::string::npos) {
    str.replace(start, from.length(), to);
    start += to.length();
  }
}

void Writer::freewrite(std::string code) {
  small_replace(code, "\\n", "\n");
  std::istringstream iss(code);
  std::string line;
  while (std::getline(iss, line)) {
    this->asm_text << "\t" << code << "\n";
  }
}

void Writer::new_global(std::string name) {
  this->asm_global << ".global " + name << "\n";
}

Writer::Writer(std::string file) {
  this->asm_out = std::ofstream(file);
  this->asm_text << ".section .text\n";
}