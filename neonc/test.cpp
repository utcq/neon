#include "lexern/lexer.h"
#include "parsern/parser.h"
#include <fstream>

int main(void) {
  std::ifstream file("style/style0x1.ne");
  if (file) {
    file.seekg(0, std::ios::end);
    int len = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = (char *)malloc(len);
    file.read(buffer, len);

    Lexer *lexer = new Lexer(buffer);
    Parser *parser = new Parser(lexer);
  }

  return 0;
}