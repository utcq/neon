#include "process.h"
#include "compn/comp.h"

int main(void) {
  std::string file = "style/style0xT.ne"; // Test
  Parser *parser = process_file(file);
  Compiler *compiler = new Compiler(&parser->ast,file);

  return 0;
}