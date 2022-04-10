#include "parsegen1.hpp"
#include <unordered_map>
#include <algorithm>

namespace {//Declarations private to this file.
template<class T> void setify(std::vector<T> &v) {
    std::sort(v.begin(), v.end());
    auto end = std::unique(v.begin(), v.end());
    v.erase(end, v.end());
    v.shrink_to_fit();
}
template<class T> std::vector<T> difference(std::vector<T> &v1, std::vector<T> &v2) {
    setify(v1); setify(v2);
    std::vector<T> result; result.resize(v1.size());
    auto end = std::set_difference(v1.cbegin(), v1.cend(), v2.cbegin(), v2.cend(), result.begin());
    result.erase(end, result.end());
    setify(result);
    return result;
}
template<class T> std::vector<T> addVectors(const std::vector<T> &v1, const std::vector<T> &v2) {
    std::vector<T> result(v1); result.resize(v1.size() + v2.size());
    std::copy(v2.cbegin(), v2.cend(), result.begin() + v1.size());
    result.shrink_to_fit();
    return result;
}
//This one requires a type argument
template<class T, class In> std::vector<T> addVectors(In v1_begin, In v1_end, In v2_begin, In v2_end) {
    std::vector<T> result(v1_begin, v1_end); const auto sz = v1_end - v1_begin;
    result.resize(sz + (v2_end - v2_begin));
    std::copy(v2_begin, v2_end, result.begin() + sz);
    result.shrink_to_fit();
    return result;
}
template<class T, class Itr> Itr bsearch_exact(Itr begin, Itr end, const T& val) {
    Itr result = std::lower_bound(begin, end, val);
    if(result == end || (*result) != val) return end;
    else return result;
}
template<class T> bool begins_with(const std::vector<T> &seq, const std::vector<T> &subseq) {
    if(subseq.size() > seq.size()) return false;
    for(size_t i = 0; i < subseq.size(); i++)
        if(seq[i] != subseq[i]) return false;
    return true;
}
#ifdef DEBUG
std::string strSubRule(const std::vector<SimpleSqlParser::ParserGeneratorPhase1::IntermediateSymbol> &subRule) {
    std::string buffer;
    if(subRule.size() < 1) return std::string("?");
    for(size_t i = 0; i < subRule.size(); i++) {
        buffer += (subRule[i].symbolType ? *(subRule[i].symbol.nonterminal) : SimpleSqlParser::TokenTypeNames[subRule[i].symbol.terminal]);
        if(i < (subRule.size() - 1)) buffer += " ";
    } 
    return buffer;
}
std::string strrule(const std::vector<std::vector<SimpleSqlParser::ParserGeneratorPhase1::IntermediateSymbol>> &rule) {
    std::string buffer;
    for(size_t i = 0; i < rule.size(); i++) {
        buffer += strSubRule(rule[i]);
        if(i < (rule.size() - 1)) buffer += " | ";
    }
    return buffer;
}
#endif
}

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

#ifdef DEBUG 
void ParserGeneratorPhase1::outputRules() {
    for(size_t i = 0; i < nonterminalArray.size(); i++)
        std::cout<<nonterminalArray[i]<<" ::= "<<strrule(rules[i])<<std::endl;
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
    setify(leftRecursionRule);
    nonLeftRecursionRule = difference(rule, leftRecursionRule);
    const std::vector<IntermediateSymbol> singleton({newNonterminal});
    for(auto& subRule: nonLeftRecursionRule) newRule.push_back(addVectors(subRule, singleton));
    setify(newRule); rules[nonterminalIndex] = newRule;
    for(auto& subRule: leftRecursionRule) newRule2.push_back(addVectors<IntermediateSymbol>(subRule.cbegin()+1, subRule.cend(), singleton.cbegin(), singleton.cend()));
    newRule2.push_back(std::vector<IntermediateSymbol>()); //Add epsilon specifier
    setify(newRule2); 
#ifdef DEBUG
    std::cout<<"New rules:\n";
    std::cout<<nonterminal<<" ::= "<<strrule(newRule)<<std::endl;
    std::cout<<newNonterminal<<" ::= "<<strrule(newRule2)<<std::endl;
#endif
    return {newNonterminal, newRule2};
}
void ParserGeneratorPhase1::removeLeftRecursion() {
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
                        newRuleSet.push_back(addVectors<IntermediateSymbol>(jSubRule.cbegin(), jSubRule.cend(), subRule.cbegin()+1, subRule.cend()));
                else newRuleSet.push_back(subRule);
            setify(newRuleSet);
            rules[i] = newRuleSet;
#ifdef DEBUG
            std::cout<<nonterminalArray[i]<<" ::= "<<strrule(newRuleSet)<<std::endl;
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
            nonterminalArray.push_back(itr->second.first);
            newRules.push_back(itr->second.second);
        };
    }
    nonterminalArray = newNonterminalArray; rules = newRules;
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
        if(begins_with(subRule, longestCommonSubsequence)) leftFactoringRule.push_back(subRule);
    setify(leftFactoringRule);
    nonLeftFactoringRule = difference(rule, leftFactoringRule);

    ruleSet1 = nonLeftFactoringRule; ruleSet1.push_back(addVectors(longestCommonSubsequence, std::vector<IntermediateSymbol>({newNonterminal})));
    setify(ruleSet1); rules[nonterminalIndex] = ruleSet1;

    for(auto& subRule : leftFactoringRule) {
        std::vector<IntermediateSymbol> slice; slice.resize(subRule.size() - longestCommonSubsequence.size());
        std::copy(subRule.cbegin()+longestCommonSubsequence.size(), subRule.cend(), slice.begin());
        ruleSet2.push_back(std::move(slice));
    } setify(ruleSet2);

#ifdef DEBUG
    std::cout<<"Longest common subsequence selected: "<<strSubRule(longestCommonSubsequence)<<std::endl;
    std::cout<<"\nNew rules:\n";
    std::cout<<nonterminal<<" ::= "<<strrule(ruleSet1)<<std::endl;
    std::cout<<newNonterminal<<" ::= "<<strrule(ruleSet2)<<std::endl;
#endif

    return {newNonterminal, ruleSet2};
}
void ParserGeneratorPhase1::leftFactoring() {
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
            const std::string &nonterminal = nonterminalArray[i];
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
                nonterminalArray.push_back(itr->second.first);
                newRules.push_back(itr->second.second);
            };
        }
        nonterminalArray = newNonterminalArray; rules = newRules;
    } while(runScan);
}
ParserGeneratorPhase1::ParserGeneratorPhase1(std::initializer_list<std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>>> cfg) {
    for(auto &rule : cfg) {
        nonterminalArray.push_back(rule.first);
        rules.push_back(rule.second);
        setify(rules.back());
    }
}


}