#include "parsegen2.hpp"
#include <utility>
#include <algorithm>
#include <sstream>
#include "setutil.hpp"
#include "error.hpp"
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
    nonterminalArray = pgp1.nonterminalArray;
#else
    nonterminalArray = std::move(pgp1.nonterminalArray); //We do not know if move or copy?
#endif
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
            newSubRule.shrink_to_fit(); return newSubRule;
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
std::vector<TokenType> compositeFirstSet(std::vector<Symbol>::const_iterator begin, 
    std::vector<Symbol>::const_iterator end, const std::vector<std::vector<TokenType>> &first) {
    std::vector<TokenType> result;
    for(std::vector<Symbol>::const_iterator itr = begin; itr != end; itr++) {
        const auto& symbol = *itr; 
        if(symbol.symbolType) {//Is nonterminal
            const auto& symbolFirst = first[symbol.symbol.nonterminalIndex];
            result.insert(result.end(), symbolFirst.begin(), symbolFirst.end()); SetUtil::setify(result);
            if(SetUtil::bsearch(symbolFirst.begin(), symbolFirst.end(), NONE) == symbolFirst.end()) {
                auto pos = SetUtil::bsearch(result.begin(), result.end(), NONE);
                if(pos != result.end()) result.erase(pos);
                break;
            }
        } else {
            result.push_back(symbol.symbol.terminal); SetUtil::setify(result);
            if(symbol.symbol.terminal != NONE) {
                auto pos = SetUtil::bsearch(result.begin(), result.end(), NONE);
                if(pos != result.end()) result.erase(pos);
                break;
            }
        }
    }
    SetUtil::setify(result); return result;
}
void ParserGeneratorPhase2::generateFollowSets() {
    generateFirstSets(); //Required
    follow.resize(nonterminalArray.size());
    follow[0] = std::vector<TokenType>({EOI});
    bool runScan;
    do {
        runScan = false;
        for(size_t k = 0; k < nonterminalArray.size(); k++) {
            const std::string &nonterminal = nonterminalArray[k];
            const auto& rule = rules[k];
            for(const auto& subRule : rule) {
                for(size_t i = 0; i < subRule.size(); i++) {
                    const auto& targetSymbol = subRule[i];
                    if(!targetSymbol.symbolType) continue; //If not a nonterminal
                    std::vector<TokenType> newSet(follow[targetSymbol.symbol.nonterminalIndex]); //explicit copy required
                    if(newSet.empty()) runScan = true;
                    
                    const auto betaSet = compositeFirstSet(subRule.begin()+i+1, subRule.end(), first);
                    newSet.insert(newSet.end(), betaSet.begin(), betaSet.end()); SetUtil::setify(newSet);
                    auto temp_itr = SetUtil::bsearch(newSet.begin(), newSet.end(), NONE);
                    if(temp_itr != newSet.end()) newSet.erase(temp_itr);
                    
                    const auto &alphaSet = follow[k]; //explicit copy required
                    if(betaSet.empty() || SetUtil::bsearch(betaSet.begin(), betaSet.end(), NONE) != betaSet.end()) 
                        {newSet.insert(newSet.end(), alphaSet.begin(), alphaSet.end()); SetUtil::setify(newSet);}
                    
                    SetUtil::setify(newSet);
                    if(newSet != follow[targetSymbol.symbol.nonterminalIndex]) runScan = true;
                    follow[targetSymbol.symbol.nonterminalIndex] = std::move(newSet);
                }
            }
        }
    } while (runScan);
#ifdef DEBUG 
    std::cout<<"\nFollow sets:\n";
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
        std::cout<<nonterminalArray[i]<<" -> ";
        for(auto& symb: follow[i]) std::cout<<TokenTypeNames[symb]<<" ";
        std::cout<<"\n";
    }
#endif 
}
void ParserGeneratorPhase2::checkIfLL1() {
    generateFollowSets(); //required
    std::stringstream err(std::stringstream::out);
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
        const std::string &nonterminal = nonterminalArray[i];
        const auto& rule = rules[i];
        if(SetUtil::bsearch(rule.begin(), rule.end(), std::vector<Symbol>()) != rule.end() &&
            !SetUtil::isdisjoint(first[i].begin(), first[i].end(), follow[i].begin(), follow[i].end())) {
            err<<"Grammar is not LL(1); for nonterminal "<<nonterminal
                <<"\nThe nonterminal produces an empty string and first and follow sets are not disjoint.";
#ifdef DEBUG 
            err<<"\nFirst set: ";
            for(auto& symb : first[i]) err<<TokenTypeNames[symb]<<" ";
            err<<"\nFollow set: ";
            for(auto& symb : follow[i]) err<<TokenTypeNames[symb]<<" ";
#endif
            throw SyntaxError(err.str());
        }
        for(size_t j = 0; j < rule.size()-1; j++) for(size_t k = j+1; k < rule.size(); k++) {
            const auto& prod1 = rule[j], prod2 = rule[k];
            if(prod1.empty() || prod2.empty()) continue;
            const auto prod1First = compositeFirstSet(prod1.begin(), prod1.end(), first);
            const auto prod2First = compositeFirstSet(prod2.begin(), prod2.end(), first);
            if(SetUtil::isdisjoint(prod1First.begin(), prod1First.end(), prod2First.begin(), prod2First.end())) continue;
            err<<"Grammar is not LL(1); for nonterminal"<<nonterminal
                <<"\nFirst sets for two of the productions of the nonterminal are not disjoint.";
#ifdef DEBUG 
            err<<"\nFor productions:\n";
            err<<nonterminal<<" ::= "<<strSubRule(prod1, nonterminalArray)<<"; first set = ";
            for(auto& symb : prod1First) err<<TokenTypeNames[symb]<<" ";
            err<<"\n"<<nonterminal<<" ::= "<<strSubRule(prod2, nonterminalArray)<<"; first set = ";
            for(auto& symb : prod2First) err<<TokenTypeNames[symb]<<" ";
#endif
            throw SyntaxError(err.str());
        }
    }
}
}

#include "setutil.cpp"