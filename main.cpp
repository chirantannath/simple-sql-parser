#include <iostream>
#include <fstream>
#include "lexer.hpp"
#include "error.hpp"

int main(int argc, char *argv[]) {
    ++argv; --argc;
    std::istream *src; std::ifstream fsrc;
    if(argc > 0) {fsrc.open(argv[0]); src = &fsrc;} else src = &std::cin;
#ifdef DEBUG
    SimpleSqlParser::Lexer lex(src);
    do {
        lex.getNextToken(); 
        lex.showstatus();
    } while (lex.getCurrentToken() != SimpleSqlParser::EOI);
#endif
    if(argc > 0) {fsrc.close();}
}