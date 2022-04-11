#include <iostream>
#include <fstream>
#include "error.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
    try {
        ++argv; --argc;
        std::ifstream file;
        if(argc > 0) file.open(argv[0]);
        std::istream *ptr = (argc > 0) ? &file : &std::cin;
        SimpleSqlParser::Parser parser(*ptr);
        int errorFlag = 0;
        while(true) {
            try {
                parser.continueParse();
            } catch (SimpleSqlParser::SyntaxError ex) {
                errorFlag = 1;
                std::cerr<<"\n"<<ex.what()<<"\n";
                if(parser.unrecoverable) break;
                continue;
            }
            break;
        }
        return errorFlag;
    } catch (SimpleSqlParser::SyntaxError ex) {
        std::cerr<<"\n"<<ex.what()<<"\n";
        return 1;
    }
}