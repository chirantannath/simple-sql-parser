#include <iostream>
#include <fstream>
#include "error.hpp"
#include "parser.hpp"

int forFile(std::istream *file, const char *fname, SimpleSqlParser::Parser *parser) {
    int errorFlag = 0;
    parser->reopen(file);
    while(true) {
        try {
            parser->continueParse();
        } catch (SimpleSqlParser::SyntaxError &ex) {
            errorFlag = 1;
            std::cerr<<"\nFrom "<<fname<<": "<<ex.what()<<"\n";
#ifdef DEBUG 
            if(parser->unrecoverable) std::cout<<"True\n"; else std::cout<<"False\n";
#endif
            if(parser->unrecoverable) break;
            continue;
        }
        break;
    }
    return errorFlag;
}

int main(int argc, char *argv[]) {
    SimpleSqlParser::Parser *parser = nullptr;
#ifdef DEBUG
    try {
#endif
        parser = new SimpleSqlParser::Parser;
#ifdef DEBUG
    }
    catch(SimpleSqlParser::SyntaxError &ex) {
        std::cerr<<"SQL Definition error: "<<ex.what()<<"\n";
        return 1;
    }
#endif
    int errorSum = 0;
    ++argv, --argc;
    if(argc == 0) errorSum = forFile(&std::cin, "<standard input>", parser);
    else {
        std::ifstream file;
        for(; argc > 0; ++argv, --argc) {
            file.open(argv[0]);
            errorSum += forFile(&file, argv[0], parser);
            file.close();
        }
    }
    delete parser; return errorSum;
}