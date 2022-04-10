#ifndef __PARSER__
#define __PARSER__

#include <vector>
#include <cstddef>
#include <functional>
#include "lexer.hpp"

namespace SimpleSqlParser {
//This needs to be hashable.
struct Symbol {
    union {
        TokenType terminal;
        size_t nonterminalIndex;
    } symbol;
    unsigned symbolType : 1; //1 or true value if nonterminal

    constexpr bool operator==(const Symbol &symb) const noexcept {
        return symbolType == symb.symbolType && 
            (symbolType ? symbol.nonterminalIndex == symb.symbol.nonterminalIndex : symbol.terminal == symb.symbol.terminal);
    }
};

enum ErrorRecovery : unsigned char {POP, SCAN};

struct ParsingTableEntry {
    union {
        ErrorRecovery recoveryAction;
        size_t subRuleIndex;
        size_t &stackAction() noexcept {return subRuleIndex;} //type alias
    } action;
    unsigned actionType : 1; //1 or true value if stackAction
};

typedef std::vector<std::vector<ParsingTableEntry>> ParsingTableMatrixType;
typedef std::vector<std::vector<Symbol>> ruleType;
typedef std::pair<size_t, ruleType> fullRuleType;

}

namespace std {
template<> struct hash<SimpleSqlParser::Symbol> {
    constexpr size_t operator()(const SimpleSqlParser::Symbol &symb) const noexcept {
        return symb.symbolType ? symb.symbol.nonterminalIndex << 1 : ((size_t)symb.symbol.terminal << 1) + 1;
    }
};
}

#endif