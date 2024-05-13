#include "handler/handler.h"
#include "lexern/lexer.h"
#include "parsern/parser.h"

#include <cstring>
#include <fstream>

Parser *process_file(std::string path) {
  FileError *error = new FileError();
  std::ifstream file(path);
  if (file) {
    file.seekg(0, std::ios::end);
    int len = file.tellg();
    file.seekg(0, std::ios::beg);
    char *buffer = (char *)malloc(len);
    file.read(buffer, len);

    std::string xty = std::string(buffer, len); // Workaround for read problems
    const char *buff = xty.c_str();

    Lexer *lexer = new Lexer(buff,path);
    Parser *parser = new Parser(lexer,path);
    return parser;
  }
  error->MissingFile(path);
  delete error;
  return nullptr;
}