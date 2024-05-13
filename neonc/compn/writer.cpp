#include "writer.h"
#include "common.h"
#include "comp.h"
#include <array>
#include <ios>
#include <sstream>
#include <utility>

std::pair<std::string, std::array<std::string, 4>> regs_transmap[14] = {
    {"rax", {"ah", "ax", "eax", "rax"}},
    {"rbx", {"bh", "bx", "ebx", "rbx"}},
    {"rcx", {"ch", "cx", "ecx", "rcx"}},
    {"rdx", {"dh", "dx", "edx", "rdx"}},
    {"rsi", {"sil", "si", "esi", "rsi"}},
    {"rdi", {"dil", "di", "edi", "rdi"}},
    {"r8", {"r8b", "r8w", "r8d", "r8"}},
    {"r9", {"r9b", "r9w", "r9d", "r9"}},
    {"r10", {"r10b", "r10w", "r10d", "r10"}},
    {"r11", {"r11b", "r11w", "r11d", "r11"}},
    {"r12", {"r12b", "r12w", "r12d", "r12"}},
    {"r13", {"r13b", "r13w", "r13d", "r13"}},
    {"r14", {"r14b", "r14w", "r14d", "r14"}},
    {"r15", {"r15b", "r15w", "r15d", "r15"}}};

Translation translate_insword(std::string instruction, std::string reg,
                              uint size) {

  std::pair<std::string, std::array<std::string, 4>> map_entry;
  for (std::pair<std::string, std::array<std::string, 4>> entry :
       regs_transmap) {
    if (entry.first == reg) {
      map_entry = entry;
      break;
    }
  }

  if (size == BYTE) {
    return {instruction + "b", {map_entry.second.at(0)}};
  } else if (size == WORD) {
    return {instruction + "w", {map_entry.second.at(1)}};
  } else if (size == DWORD) {
    return {instruction + "d", {map_entry.second.at(2)}};
  } else if (size == QWORD) {
    return {instruction + "q", {map_entry.second.at(3)}};
  }
  return {};
}

// Class

Writer::Writer(std::string file) {
  this->asm_out = std::ofstream(file);
  this->asm_global << ".intel_syntax noprefix\n";
  this->asm_text << ".section .text\n";
  this->asm_rodata << ".section .rodata\n";
}

void Writer::end() {
  this->asm_out << this->asm_global.str();
  this->asm_out << this->asm_rodata.str() << "\n";
  this->asm_out << this->asm_data.str() << "\n";
  this->asm_out << this->asm_text.str() << "\n";
  this->asm_out.close();
}

// MNG
void Writer::label(std::string name) { this->asm_text << name << ":\n"; }

void Writer::new_global(std::string name) {
  this->asm_global << ".global " + name << "\n";
}

void Writer::new_string(std::string name, std::string value) {
  this->asm_rodata << name << ":\n\t.string \"" << value << "\"\n";
}

// Utils
void Writer::freewrite(std::string code) {
  small_replace(code, "\\n", "\n");
  std::istringstream iss(code);
  std::string line;
  while (std::getline(iss, line)) {
    this->asm_text << "\t" << code << "\n";
  }
}

void Writer::function_prologue() {
  this->push(RBP);
  this->move(RBP, RSP);
  this->freewrite("sub rsp, @STACK_SIZE");
}

void Writer::function_epilogue() { this->freewrite("leave"); }

void Writer::replace_free(std::string from, std::string to) {
  std::string new_asm_text = this->asm_text.str();
  small_replace(new_asm_text, from, to);
  this->asm_text.str("");
  this->asm_text << new_asm_text;
}

// Instructions
void Writer::push(std::string src_register) {
  this->asm_text << "\tpush " << src_register << "\n";
}

void Writer::move(std::string dst_register, std::string src_register) {
  this->asm_text << "\tmov " << dst_register << ", " << src_register << "\n";
}

void Writer::move(std::string dst_register, std::string src_label, uint size) {
  Translation dstt = translate_insword("mov", dst_register, size);
  this->asm_text << "\t" << dstt.mnemonic << " " << dstt.reg_sz << ", OFFSET "
                 << src_label << "\n";
}

void Writer::move(std::string dst_register, int64_t primitive) {
  this->asm_text << "\tmov " << dst_register << ", " << primitive << "\n";
}

void Writer::move(Pointer *dst_ptr, std::string src_register, uint size) {
  Translation srct = translate_insword("mov", src_register, size);
  this->asm_text << "\t" << srct.mnemonic << " [" << dst_ptr->reg
                 << std::showpos << dst_ptr->offset << "], " << srct.reg_sz
                 << "\n";
}

void Writer::move(std::string dst_register, Pointer *src_ptr, uint size) {
  Translation dstt = translate_insword("mov", dst_register, size);
  this->asm_text << "\t" << dstt.mnemonic << " " << dstt.reg_sz << ", ["
                 << src_ptr->reg << std::showpos << src_ptr->offset << "]\n";
}

void Writer::xorr(std::string dst_register, std::string src_register) {
  this->asm_text << "\txor " << dst_register << ", " << src_register << "\n";
}

void Writer::pop(std::string dst_register) {
  this->asm_text << "\tpop " << dst_register << "\n";
}

void Writer::ret() { this->asm_text << "\tret\n"; }

void Writer::call(std::string name) {
  this->asm_text << "\tcall " << name << "\n";
}

// String Utils
void small_replace(std::string &str, const std::string &from,
                   const std::string &to) {
  size_t start = 0;
  while ((start = str.find(from, start)) != std::string::npos) {
    str.replace(start, from.length(), to);
    start += to.length();
  }
}
