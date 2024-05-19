#include "handler.h"
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <string>

void print_line(std::string file, int line) {
    // Horrible but I'm too lazy to create a new class for this
    std::ifstream input(file);
    std::string content;
    int current = 1;

    while (std::getline(input, content)) {
        if (current == line) {
            content.erase(0, content.find_first_not_of(" "));
            content.erase(0, content.find_first_not_of("\t"));
            std::cout << "  " << line << " | " << content << std::endl;
            break;
        }
        current++;
    }
    input.close();
}

LexerError::LexerError() { this->err_flag=false; }

LexerError::~LexerError() {
    if (this->err_flag) { exit(EXIT_FAILURE); }
}

void LexerError::UnknownSymbol(char symbol, Pos pos) {
    std::cerr << pos.file << ":" << pos.line << ":" << pos.col << ": [❓] Unknown Symbol: '" << symbol << "' \n";
    print_line(pos.file, pos.line);
    this->err_flag = true;
}


FileError::FileError() { this->err_flag=false; }

FileError::~FileError() {
    if (this->err_flag) { exit(EXIT_FAILURE); }
}

void FileError::MissingFile(std::string file) {
    std::cerr << "[✳️] File '" << file << "' not found" "\n";
    this->err_flag = true;
}

ParserError::ParserError() { this->err_flag=false; }

ParserError::~ParserError() {
    if (this->err_flag) { exit(EXIT_FAILURE); }
}

void ParserError::UnknownStatement(Token *token) {
    std::cerr << token->position.file << ":" << token->position.line << ":" << token->position.col << ": [❓] Unknown Statement: '" << token->value << "' \n";
    print_line(token->position.file, token->position.line);
    this->err_flag = true;
}

void ParserError::UnknownOperator(Token *token) {
    std::cerr << token->position.file << ":" << token->position.line << ":" << token->position.col << ": [❓] Unknown Operator: '" << token->value << "' \n";
    print_line(token->position.file, token->position.line);
    this->err_flag = true;
}

void ParserError::UnknownWhat(Token *token) {
    std::cerr << token->position.file << ":" << token->position.line << ":" << token->position.col << ": [🔥] What am i supposed to do with this: '" << token->value << "' \n";
    print_line(token->position.file, token->position.line);
    this->err_flag = true;
}

void ParserError::MissingToken(std::string token, Pos pos) {
    std::cerr << pos.file << ":" << pos.line << ":" << pos.col << ": [🔍] Missing Token: '" << token << "' in the statement.\n";
    print_line(pos.file, pos.line);
    this->err_flag = true;
}

void ParserError::UnexpectedToken(Token *token, std::string expected) {
    std::cerr << token->position.file << ":" << token->position.line << ":" << token->position.col << ": [🔍] Unexpected Token: '" << token->value << "' expected '" << expected << "' \n";
    print_line(token->position.file, token->position.line);
    this->err_flag = true;
}

CompilerError::CompilerError() { this->err_flag=false; }
CompilerError::~CompilerError() {
    if (this->err_flag) { exit(EXIT_FAILURE); }
}

void CompilerError::VariableNotFound(std::string name, std::string file) {
    std::cerr << file << ":?:?: [⚠️] Variable not found: '" << name << "' \n";
    this->err_flag = true;
}

void CompilerError::PrivateProcedure(std::string name, std::string file) {
    std::cerr << file << ":?:?: [🚫] Private Procedure: '" << name << "' cannot resolve it. \n";
    this->err_flag = true;
}

void CompilerError::ProcedureNotFound(std::string name, std::string file) {
    std::cerr << file << ":?:?: [❌] Procedure '" << name << "' not found in scope. \n";
    this->err_flag = true;
}

void CompilerError::TypeResolveError(std::string name, std::string ext, std::string file) {
    std::cerr << file << ":?:?: [❌] Type '" << name << "' cannot be resolved in '" << ext << "' \n";
    this->err_flag = true;
}

void CompilerError::ProcedureRedefined(std::string name, std::string file) {
    std::cerr << file << ":?:?: [⛔] Procedure '" << name << "' already defined. \n";
    this->err_flag = true;
}