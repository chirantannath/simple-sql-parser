#include "parser.hpp"

namespace SimpleSqlParser {
#ifdef DEBUG
std::string strSubRule(const std::vector<Symbol> &subRule, const std::vector<std::string> &nonterminalArray) {
    std::string buffer;
    if(subRule.size() < 1) return std::string("?");
    for(size_t i = 0; i < subRule.size(); i++) {
        buffer += (subRule[i].symbolType ? nonterminalArray[subRule[i].symbol.nonterminalIndex] : TokenTypeNames[subRule[i].symbol.terminal]);
        if(i < (subRule.size() - 1)) buffer += " ";
    } 
    return buffer;
}
std::string strrule(const std::vector<std::vector<Symbol>> &rule, const std::vector<std::string> &nonterminalArray) {
    std::string buffer;
    for(size_t i = 0; i < rule.size(); i++) {
        buffer += strSubRule(rule[i], nonterminalArray);
        if(i < (rule.size() - 1)) buffer += " | ";
    }
    return buffer;
} 
#endif
}
