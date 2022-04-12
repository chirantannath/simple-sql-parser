#include "parsegen1.hpp"
#include <unordered_map>
#include <algorithm>
#include "setutil.hpp"

#ifdef DEBUG
namespace {//Declarations private to this file.
std::string _strSubRule(const std::vector<SimpleSqlParser::ParserGeneratorPhase1::IntermediateSymbol> &subRule) {
    std::string buffer;
    if(subRule.size() < 1) return std::string("?");
    for(size_t i = 0; i < subRule.size(); i++) {
        buffer += (subRule[i].symbolType ? *(subRule[i].symbol.nonterminal) : SimpleSqlParser::TokenTypeNames[subRule[i].symbol.terminal]);
        if(i < (subRule.size() - 1)) buffer += " ";
    } 
    return buffer;
}
std::string _strrule(const std::vector<std::vector<SimpleSqlParser::ParserGeneratorPhase1::IntermediateSymbol>> &rule) {
    std::string buffer;
    for(size_t i = 0; i < rule.size(); i++) {
        buffer += _strSubRule(rule[i]);
        if(i < (rule.size() - 1)) buffer += " | ";
    }
    return buffer;
}}
#endif


namespace SimpleSqlParser {
//ParserGeneratorPhase1
ParserGeneratorPhase1::IntermediateSymbol::IntermediateSymbol(const IntermediateSymbol &symb) {
    symbolType = symb.symbolType;
    if(symbolType) symbol.nonterminal = new std::string(*symb.symbol.nonterminal);
    else symbol.terminal = symb.symbol.terminal;
}
ParserGeneratorPhase1::IntermediateSymbol::IntermediateSymbol(IntermediateSymbol&& symb) noexcept {
    symbolType = symb.symbolType;
    if(symbolType) {
        symbol.nonterminal = symb.symbol.nonterminal;
        symb.symbol.nonterminal = nullptr;
        symb.symbol.terminal = NONE;
        symb.symbolType = 0;
    }
    else symbol.terminal = symb.symbol.terminal;
}
ParserGeneratorPhase1::IntermediateSymbol::~IntermediateSymbol() noexcept {
    if(symbolType && symbol.nonterminal) delete symbol.nonterminal;
    symbol.nonterminal = nullptr;
}
ParserGeneratorPhase1::IntermediateSymbol& ParserGeneratorPhase1::IntermediateSymbol::operator=(const IntermediateSymbol &symb) {
    if(symbolType && symbol.nonterminal) delete symbol.nonterminal;
    symbolType = symb.symbolType;
    if(symbolType) symbol.nonterminal = new std::string(*symb.symbol.nonterminal);
    else symbol.terminal = symb.symbol.terminal;
    return *this;
}
ParserGeneratorPhase1::IntermediateSymbol& ParserGeneratorPhase1::IntermediateSymbol::operator=(const std::string &nonterminal) {
    if(symbolType && symbol.nonterminal) delete symbol.nonterminal;
    symbolType = 1;
    symbol.nonterminal = new std::string(nonterminal);
    return *this;
}
ParserGeneratorPhase1::IntermediateSymbol& ParserGeneratorPhase1::IntermediateSymbol::operator=(TokenType terminal) noexcept {
    if(symbolType && symbol.nonterminal) delete symbol.nonterminal;
    symbolType = 0;
    symbol.terminal = terminal;
    return *this;
}
ParserGeneratorPhase1::IntermediateSymbol& ParserGeneratorPhase1::IntermediateSymbol::operator=(IntermediateSymbol&& symb) noexcept {
    if(symbolType && symbol.nonterminal) delete symbol.nonterminal;
    symbolType = symb.symbolType;
    if(symbolType) {
        symbol.nonterminal = symb.symbol.nonterminal;
        symb.symbol.nonterminal = nullptr;
        symb.symbol.terminal = NONE;
        symb.symbolType = 0;
    }
    else symbol.terminal = symb.symbol.terminal;
    return *this;
}
int ParserGeneratorPhase1::IntermediateSymbol::compare(const IntermediateSymbol& symb) const noexcept {
    if(symbolType < symb.symbolType) return -1;
    else if(symbolType > symb.symbolType) return 1;
    else if(symbolType) return symbol.nonterminal->compare(*symb.symbol.nonterminal);
    else {
        if(symbol.terminal < symb.symbol.terminal) return -1;
        else if(symbol.terminal > symb.symbol.terminal) return 1;
        else return 0;
    }
}

#ifdef DEBUG 
void ParserGeneratorPhase1::outputRules() {
    for(size_t i = 0; i < nonterminalArray.size(); i++)
        std::cout<<nonterminalArray[i]<<" ::= "<<_strrule(rules[i])<<std::endl;
}
#endif

//IntermediateSymbolHash
std::hash<std::string> ParserGeneratorPhase1::IntermediateSymbolHash::hashObject;

std::pair<std::string, std::vector<std::vector<ParserGeneratorPhase1::IntermediateSymbol>>>
ParserGeneratorPhase1::removeLeftRecursion(size_t nonterminalIndex) {
    const std::string &nonterminal = nonterminalArray[nonterminalIndex];
    std::string newNonterminal; size_t nameGen = 1;
    do {
        newNonterminal = nonterminal + "_r" + std::to_string(nameGen); nameGen++;
    } while (std::find(nonterminalArray.cbegin(), nonterminalArray.cend(), newNonterminal) != nonterminalArray.cend());
#ifdef DEBUG
    std::cout<<"\nRemoving immediate-left recursion for "<<nonterminal<<". New nonterminal will be "<<newNonterminal<<".\n";
#endif
    std::vector<std::vector<IntermediateSymbol>> leftRecursionRule, nonLeftRecursionRule, newRule, newRule2;
    std::vector<std::vector<IntermediateSymbol>> &rule = rules[nonterminalIndex];
    for(auto &subRule : rule) {
        if(!(subRule.size() > 0 && subRule[0] == nonterminal)) continue;
        leftRecursionRule.push_back(subRule);
    }
    if(leftRecursionRule.size() < 1) {
#ifdef DEBUG 
        std::cout<<"This is not left-recursive.\n";
#endif
        return {}; //Calls default ctor
    }
    SetUtil::setify(leftRecursionRule);
    nonLeftRecursionRule = SetUtil::difference(rule, leftRecursionRule);
    const std::vector<IntermediateSymbol> singleton({newNonterminal});
    for(auto& subRule: nonLeftRecursionRule) newRule.push_back(SetUtil::addVectors(subRule, singleton));
    SetUtil::setify(newRule); rules[nonterminalIndex] = newRule;
    for(auto& subRule: leftRecursionRule) newRule2.push_back(SetUtil::addVectors<IntermediateSymbol>(subRule.cbegin()+1, subRule.cend(), singleton.cbegin(), singleton.cend()));
    newRule2.push_back(std::vector<IntermediateSymbol>()); //Add epsilon specifier
    SetUtil::setify(newRule2); 
#ifdef DEBUG
    std::cout<<"New rules:\n";
    std::cout<<nonterminal<<" ::= "<<_strrule(newRule)<<std::endl;
    std::cout<<newNonterminal<<" ::= "<<_strrule(newRule2)<<std::endl;
#endif
    return {newNonterminal, newRule2};
}
void ParserGeneratorPhase1::removeLeftRecursion() {
    if(leftRecursionRemovalDone) return;
    std::unordered_map<size_t, std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>>> newNonterminals; 
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
#ifdef DEBUG
        std::cout<<"\nRemoving chained-left recursion for "<<nonterminalArray[i]<<std::endl;
#endif
        for(size_t j = 0; j < i; j++) {
#ifdef DEBUG
            std::cout<<"Trying substituting in "<<nonterminalArray[j]<<std::endl;
#endif
            std::vector<std::vector<IntermediateSymbol>> newRuleSet;
            for(auto& subRule : rules[i])
                if(subRule.size() > 0 && subRule[0] == nonterminalArray[j])
                    for(auto& jSubRule : rules[j])
                        newRuleSet.push_back(SetUtil::addVectors<IntermediateSymbol>(jSubRule.cbegin(), jSubRule.cend(), subRule.cbegin()+1, subRule.cend()));
                else newRuleSet.push_back(subRule);
            SetUtil::setify(newRuleSet);
            rules[i] = newRuleSet;
#ifdef DEBUG
            std::cout<<nonterminalArray[i]<<" ::= "<<_strrule(newRuleSet)<<std::endl;
#endif
        }
        const auto newNonterminal = removeLeftRecursion(i);
        if(newNonterminal.first.size() > 0)
            newNonterminals[i] = newNonterminal; //Insert newNonterminal after index i
    }

    //Regenerate nonterminalArray
    decltype(nonterminalArray) newNonterminalArray;
    decltype(rules) newRules;
    for(size_t i = 0; i < nonterminalArray.size(); i++) {
        newNonterminalArray.push_back(nonterminalArray[i]);
        newRules.push_back(rules[i]);
        auto itr = newNonterminals.find(i);
        if(itr != newNonterminals.end()) {
            newNonterminalArray.push_back(itr->second.first);
            newRules.push_back(itr->second.second);
        };
    }
    nonterminalArray = std::move(newNonterminalArray); rules = std::move(newRules);
    //Do not shrink. Phase not complete.
    leftRecursionRemovalDone = true;
}
std::pair<std::string, std::vector<std::vector<ParserGeneratorPhase1::IntermediateSymbol>>>
ParserGeneratorPhase1::leftFactoring(size_t nonterminalIndex) {
    const std::string &nonterminal = nonterminalArray[nonterminalIndex];
    std::string newNonterminal; size_t nameGen = 1;
    std::vector<std::vector<IntermediateSymbol>> &rule = rules[nonterminalIndex], 
        leftFactoringRule, nonLeftFactoringRule, ruleSet1, ruleSet2;
    do {
        newNonterminal = nonterminal + "_f" + std::to_string(nameGen); nameGen++;
    } while(std::find(nonterminalArray.cbegin(), nonterminalArray.cend(), newNonterminal) != nonterminalArray.cend());
#ifdef DEBUG 
    std::cout<<"\nPerforming left-factoring for "<<nonterminal<<". New nonterminal will be "<<newNonterminal<<".\n";
#endif
    std::vector<IntermediateSymbol> longestCommonSubsequence;
    for(size_t i = 0; i < rule.size(); i++) {
        std::vector<IntermediateSymbol> &seq1 = rule[i];
        for(size_t j = i+1; j < rule.size(); j++) {
            std::vector<IntermediateSymbol> commonSubsequence, &seq2 = rule[j];
            for(size_t k = 0; k < seq1.size() && k < seq2.size(); k++)
                if(seq1[k] == seq2[k]) commonSubsequence.push_back(seq1[k]);
                else break;
            if(commonSubsequence.size() > longestCommonSubsequence.size()) longestCommonSubsequence = std::move(commonSubsequence);
        }
    }
    if(longestCommonSubsequence.empty()) {
#ifdef DEBUG
        std::cout<<"Left-factoring not required.\n";
#endif
        return {};
    }

    for(auto& subRule : rule)
        if(SetUtil::begins_with(subRule, longestCommonSubsequence)) leftFactoringRule.push_back(subRule);
    SetUtil::setify(leftFactoringRule);
    nonLeftFactoringRule = SetUtil::difference(rule, leftFactoringRule);

    ruleSet1 = nonLeftFactoringRule; ruleSet1.push_back(SetUtil::addVectors(longestCommonSubsequence, std::vector<IntermediateSymbol>({newNonterminal})));
    SetUtil::setify(ruleSet1); rules[nonterminalIndex] = ruleSet1;

    for(auto& subRule : leftFactoringRule) {
        std::vector<IntermediateSymbol> slice; slice.resize(subRule.size() - longestCommonSubsequence.size());
        std::copy(subRule.cbegin()+longestCommonSubsequence.size(), subRule.cend(), slice.begin());
        ruleSet2.push_back(std::move(slice));
    } SetUtil::setify(ruleSet2);

#ifdef DEBUG
    std::cout<<"Longest common subsequence selected: "<<_strSubRule(longestCommonSubsequence)<<std::endl;
    std::cout<<"\nNew rules:\n";
    std::cout<<nonterminal<<" ::= "<<_strrule(ruleSet1)<<std::endl;
    std::cout<<newNonterminal<<" ::= "<<_strrule(ruleSet2)<<std::endl;
#endif

    return {newNonterminal, ruleSet2};
}
void ParserGeneratorPhase1::leftFactoring() {
    if(leftFactoringDone) return;
    //Multiple runs for left-factoring.
    bool runScan; 
#ifdef DEBUG
    unsigned long long run = 0;
#endif
    do {
        runScan = false;
#ifdef DEBUG
        run++; std::cout<<"\nRun "<<run<<std::endl;
#endif
        std::unordered_map<size_t, std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>>> newNonterminals; 
        for(size_t i = 0; i < nonterminalArray.size(); i++) {
            //const std::string &nonterminal = nonterminalArray[i];
            const auto newNonterminal = leftFactoring(i);
            if(newNonterminal.second.size() > 0) {
                newNonterminals[i] = newNonterminal;
                runScan = true;
            }
        }
        //Regenerate nonterminalArray
        decltype(nonterminalArray) newNonterminalArray;
        decltype(rules) newRules;
        for(size_t i = 0; i < nonterminalArray.size(); i++) {
            newNonterminalArray.push_back(nonterminalArray[i]);
            newRules.push_back(rules[i]);
            auto itr = newNonterminals.find(i);
            if(itr != newNonterminals.end()) {
                newNonterminalArray.push_back(itr->second.first);
                newRules.push_back(itr->second.second);
            };
        }
        nonterminalArray = std::move(newNonterminalArray); rules = std::move(newRules);
    } while(runScan);
    nonterminalArray.shrink_to_fit(); rules.shrink_to_fit();
    leftFactoringDone = true;
}
ParserGeneratorPhase1::ParserGeneratorPhase1(std::initializer_list<std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>>> cfg) 
: leftRecursionRemovalDone(false), leftFactoringDone(false) {
    for(auto &rule : cfg) {
        nonterminalArray.push_back(rule.first);
        rules.push_back(rule.second);
        SetUtil::setify(rules.back());
    }
}}

//We include cpp because templates need their implementation.
#include "setutil.cpp" 