#include "parsegen2.hpp"
#include <utility>
#include <algorithm>
#include "setutil.hpp"
#ifdef DEBUG
#include <iostream>
#endif

namespace SimpleSqlParser {
//ParserGeneratorPhase2
#ifdef DEBUG
void ParserGeneratorPhase2::outputRules() {
    for(size_t i = 0; i < nonterminalArray.size(); i++)
        std::cout<<nonterminalArray[i]<<" ::= "<<strrule(rules[i], nonterminalArray)<<std::endl;
}
#endif
void ParserGeneratorPhase2::init(ParserGeneratorPhase1 &pgp1) {
#ifdef DEBUG
    pgp1.outputRules();
#endif
    pgp1.removeLeftRecursion();
    pgp1.leftFactoring();
#ifdef DEBUG 
    pgp1.outputRules();
#endif
    nonterminalArray = pgp1.nonterminalArray; //We do not know if move or copy?
    rules.resize(pgp1.rules.size());
    std::transform(pgp1.rules.cbegin(), pgp1.rules.cend(), rules.begin(), [&](const std::vector<std::vector<ParserGeneratorPhase1::IntermediateSymbol>> &rule) {
        std::vector<std::vector<Symbol>> newRule; newRule.resize(rule.size());
        std::transform(rule.cbegin(), rule.cend(), newRule.begin(), [&](const std::vector<ParserGeneratorPhase1::IntermediateSymbol> &subRule) {
            std::vector<Symbol> newSubRule; newSubRule.resize(subRule.size());
            std::transform(subRule.cbegin(), subRule.cend(), newSubRule.begin(), [&](const ParserGeneratorPhase1::IntermediateSymbol &isymb){
                Symbol symb;
                if((symb.symbolType = isymb.symbolType)) 
                    symb.symbol.nonterminalIndex = 
                        std::find(nonterminalArray.cbegin(), nonterminalArray.cend(), *(isymb.symbol.nonterminal)) - nonterminalArray.cbegin();
                else symb.symbol.terminal = isymb.symbol.terminal;
                return symb;
            });
            return newSubRule;
        }); SetUtil::setify(newRule);
        return newRule;
    });
}
void ParserGeneratorPhase2::generateFirstSets() {
    bool runScan;
    first.resize(nonterminalArray.size());
    do {
        runScan = false;
        for(size_t i = 0; i < nonterminalArray.size(); i++) {
            const std::string &nonterminal = nonterminalArray[i];
            std::vector<TokenType> newSet(first[i]); //These are sorted sets so efficient searches can be done.
            //Need explicit copy above otherwise it will not work.
            if(newSet.empty()) runScan = true;
            const auto& rule = rules[i];
            for(const auto& subRule : rule) {
                bool allNullable = true;
                for(const auto& subRuleSymbol : subRule) {
                    if(subRuleSymbol.symbolType) {//Is nonterminal
                        auto& subRuleSymbolFirst = first[subRuleSymbol.symbol.nonterminalIndex];
                        bool NONEfound = false;
                        for(auto& symb : subRuleSymbolFirst) 
                            if(symb != NONE) 
                                {newSet.push_back(symb); SetUtil::setify(newSet);}
                            else if(symb == NONE) NONEfound = true;
                        if(!NONEfound) {allNullable = false; break;}
                    } else if (subRuleSymbol.symbol.terminal != NONE) {
                        newSet.push_back(subRuleSymbol.symbol.terminal); SetUtil::setify(newSet);
                        allNullable = false; break;
                    } 
                }
                if(allNullable) {newSet.push_back(NONE); SetUtil::setify(newSet);}
            }
            SetUtil::setify(newSet);
            if(newSet != first[i]) runScan = true;
            first[i] = std::move(newSet);
        }
    } while(runScan);
#ifdef DEBUG 
    std::cout<<"\nFirst sets:\n";
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
        std::cout<<nonterminalArray[i]<<" -> ";
        for(auto& symb: first[i]) std::cout<<TokenTypeNames[symb]<<" ";
        std::cout<<"\n";
    }
#endif
}
}

#include "setutil.cpp"