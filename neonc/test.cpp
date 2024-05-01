#include "process.h"
#include "compn/comp.h"

int main(void) {
  std::string file = "style/style0x1.ne";
  Parser *parser = process_file(file);
  Compiler *compiler = new Compiler(&parser->ast);

  return 0;
}