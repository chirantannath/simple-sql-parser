#include "parsegen3.hpp"
#include "lexer.hpp"
#include <utility>
#include "setutil.hpp"
#ifdef DEBUG 
#include <sstream>
#include <iostream>
#include "error.hpp"
#endif

namespace SimpleSqlParser {
#ifdef DEBUG 
void ParserGeneratorPhase3::parsingTableAssign(size_t i, TokenType k, size_t subRuleIndex) {
    if(parsingTable[i][k].actionType) {
        std::stringstream err(std::stringstream::out);
        err<<"Parsing table conflict for nonterminal "<<nonterminalArray[i]<<" and terminal "<<TokenTypeNames[k]<<std::endl;
        err<<"Conflicting rules are:\n";
        err<<nonterminalArray[i]<<" ::= "<<strSubRule(rules[i][subRuleIndex], nonterminalArray)<<std::endl;
        err<<nonterminalArray[i]<<" ::= "<<strSubRule(rules[i][parsingTable[i][k].action.subRuleIndex], nonterminalArray)<<std::endl;
    }
    else parsingTable[i][k] = stackAction(subRuleIndex);
}
#endif
void ParserGeneratorPhase3::init(ParserGeneratorPhase2 &pgp2) {
#ifdef DEBUG
    pgp2.checkIfLL1();
    std::cout<<std::endl;
    pgp2.outputRules();
    nonterminalArray = pgp2.nonterminalArray;
    rules = pgp2.rules; first = pgp2.first; follow = pgp2.follow;
#else
    pgp2.generateFollowSets();
    rules = std::move(pgp2.rules);
    first = std::move(pgp2.first); follow = std::move(pgp2.follow);
#endif
}
void ParserGeneratorPhase3::generateParsingTable() {
    if(parsingTableDone) return;
#ifdef DEBUG 
    std::cout<<"\nGenerating parsing table...\n";
#endif
    parsingTable.resize(rules.size(), std::vector<ParsingTableEntry>(TokenTypes.size()));
    for(size_t i = 0; i < rules.size(); i++) {
        for(TokenType terminal : TokenTypes) {
            if(!(SetUtil::bsearch(first[i].begin(), first[i].end(), terminal) != first[i].end() || 
                (SetUtil::bsearch(first[i].begin(), first[i].end(), NONE) != first[i].end() && 
                SetUtil::bsearch(follow[i].begin(), follow[i].end(), terminal) != follow[i].end()))) 
                parsingTable[i][terminal] 
                    = (terminal == EOI || SetUtil::bsearch(follow[i].begin(), follow[i].end(), terminal) != follow[i].end()) ?
                        errorRecovery(POP) : errorRecovery(SCAN);
#ifdef DEBUG
            else parsingTable[i][terminal].actionType = 0; //NOT an action
#endif
        }
    }

    for(size_t i = 0; i < rules.size(); i++) {
        const auto& rule = rules[i];
        for(size_t subRuleIndex = 0; subRuleIndex < rule.size(); subRuleIndex++) {
            const auto &subRule = rule[subRuleIndex];
            const auto firstSet = compositeFirstSet(subRule.begin(), subRule.end(), first);
            bool NONEfound = false;
            for(TokenType terminal : firstSet) {
                parsingTableAssign(i, terminal, subRuleIndex);
                if(terminal == NONE) NONEfound = true;
            }
            if(NONEfound || firstSet.empty()) for(TokenType terminal : follow[i]) parsingTableAssign(i, terminal, subRuleIndex);
        } 
    }

#ifdef DEBUG 
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
        const auto& nonterminal = nonterminalArray[i];
        std::cout<<"\nWhen parsing "<<nonterminal<<":\n";
        for (TokenType terminal : TokenTypes) {
            std::cout<<"If current token is "<<TokenTypeNames[terminal]<<": do ";
            if(parsingTable[i][terminal].actionType)
                std::cout<<nonterminal<<" ::= "<<strSubRule(rules[i][parsingTable[i][terminal].action.subRuleIndex], nonterminalArray)<<std::endl;
            else 
                std::cout<<ErrorRecoveryNames[parsingTable[i][terminal].action.recoveryAction]<<" for error recovery\n";
        }
    }
#endif
    parsingTableDone = true;
}

}

#include "setutil.cpp"