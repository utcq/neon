#include "compn/comp.h"
#include "lexern/lexer.h"
#include "parsern/parser.h"

#include <fstream>
#include <stdexcept>

Parser *process_file(std::string path) {
  std::ifstream file(path);
  if (file) {
    file.seekg(0, std::ios::end);
    int len = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = (char *)malloc(len);
    file.read(buffer, len);

    Lexer *lexer = new Lexer(buffer);
    Parser *parser = new Parser(lexer);
    return parser;
  }
  throw std::invalid_argument("Missing file: " + path);
}