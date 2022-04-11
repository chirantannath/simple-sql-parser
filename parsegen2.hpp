#ifndef __PARSEGEN2__
#define __PARSEGEN2__

#include <vector>
#include <string>
#include <unordered_map>
#include "parsegen1.hpp"

namespace SimpleSqlParser {
//Factory class for Parser, phase 2: first and follow sets (at the end we will know if we are really LL(1))
class ParserGeneratorPhase2 {
#ifdef DEBUG 
public:
    void outputRules();
#endif
    std::vector<std::string> nonterminalArray;
    std::vector<std::vector<std::vector<Symbol>>> rules;
    std::vector<std::vector<TokenType>> first, follow;

    void init(ParserGeneratorPhase1&);
    void generateFirstSets();
    //void generateFollowSets();
    ParserGeneratorPhase2(ParserGeneratorPhase1 &pgp1) {init(pgp1);}
    ParserGeneratorPhase2() {ParserGeneratorPhase1 pgp1; init(pgp1);}
};

}

#endif