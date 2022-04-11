#include <iostream>
#include <fstream>
#include "lexer.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "parsegen3.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG 
    SimpleSqlParser::ParserGeneratorPhase3 pgp3;
    std::cout<<std::endl;
    pgp3.generateParsingTable();;
#endif
}