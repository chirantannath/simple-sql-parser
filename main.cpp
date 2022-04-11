#include <iostream>
#include <fstream>
#include "lexer.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "parsegen2.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG 
    SimpleSqlParser::ParserGeneratorPhase2 pgp2;
    pgp2.outputRules();
    pgp2.generateFirstSets();
#endif
}