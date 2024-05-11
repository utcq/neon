#ifndef COMPN_COMMON_H
#define COMPN_COMMON_H

#include <iostream>
#include <vector>

#define NONE  0
#define BYTE  1
#define WORD  2
#define DWORD 4
#define QWORD 8

struct Var {
  std::string name;
  std::string type;
  uint size;
  int stack_offset;
};

struct Pointer {
  std::string reg;
  int offset;
};

extern std::string regs_ord[12];

#endif