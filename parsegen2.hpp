#ifndef __PARSEGEN2__
#define __PARSEGEN2__

#include <vector>
#include <string>
#include "parsegen1.hpp"

namespace SimpleSqlParser {
class ParserGeneratorPhase3;
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
    void generateFollowSets();
#ifdef DEBUG
    void checkIfLL1(); //throws SyntaxException
#endif
    ParserGeneratorPhase2(ParserGeneratorPhase1 &pgp1) {init(pgp1);}
    ParserGeneratorPhase2() {ParserGeneratorPhase1 pgp1; init(pgp1);}

public:
    friend class SimpleSqlParser::ParserGeneratorPhase3;
};
//Outside because required by following phases.
std::vector<TokenType> compositeFirstSet(std::vector<Symbol>::const_iterator, 
    std::vector<Symbol>::const_iterator, const std::vector<std::vector<TokenType>>&);
}

#endif