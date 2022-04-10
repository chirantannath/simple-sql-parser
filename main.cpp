#include <iostream>
#include <fstream>
#include "lexer.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "parsegen1.hpp"

int main(int argc, char *argv[]) {
#ifdef DEBUG 
    SimpleSqlParser::ParserGeneratorPhase1 pgp1;
    pgp1.outputRules();
    pgp1.removeLeftRecursion();
    pgp1.leftFactoring();
    pgp1.outputRules();
#endif
}