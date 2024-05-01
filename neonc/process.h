#ifndef PROCESS_H
#define PROCESS_H

#include "lexern/lexer.h"
#include "parsern/parser.h"
#include "compn/comp.h"

Parser *process_file(std::string path);

#endif