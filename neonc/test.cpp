#include "process.h"
#include "compn/comp.h"

int main(void) {
  std::string file = "style/style0xT.ne"; // C_Test
  Parser *parser = process_file(file);
  Compiler *compiler = new Compiler(&parser->ast);

  return 0;
}