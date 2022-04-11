#ifndef __PARSEGEN3__
#define __PARSEGEN3__

#include <vector>
#include "parsegen2.hpp"
#ifdef DEBUG 
#include <string>
#endif

namespace SimpleSqlParser {
//Factory class for Parser, phase 3: parsing table construction.
class ParserGeneratorPhase3  {
#ifdef DEBUG 
public:
    std::vector<std::string> nonterminalArray;
    void parsingTableAssign(size_t, TokenType, size_t);
#else
    void parsingTableAssign(size_t i, TokenType k, size_t si) {parsingTable[i][k] = stackAction(si);};
#endif
    std::vector<std::vector<std::vector<Symbol>>> rules;
    std::vector<std::vector<TokenType>> first, follow;
    std::vector<std::vector<ParsingTableEntry>> parsingTable;

    void init(ParserGeneratorPhase2&);
    void generateParsingTable();
    ParserGeneratorPhase3(ParserGeneratorPhase2 &pgp2) {init(pgp2);}
    ParserGeneratorPhase3() {ParserGeneratorPhase2 pgp2; init(pgp2);}
public:
    friend class SimpleSqlParser::Parser;
};
}

#endif