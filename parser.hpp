#ifndef __PARSER__
#define __PARSER__

#include "lexer.hpp"
#ifdef DEBUG
#include <string>
#endif

namespace SimpleSqlParser {
//This needs to be hashable.
struct Symbol {
    union {
        TokenType terminal;
        size_t nonterminalIndex;
    } symbol;
    unsigned symbolType : 1; //1 or true value if nonterminal

    bool operator==(const Symbol &symb) const noexcept {
        return symbolType == symb.symbolType && 
            (symbolType ? symbol.nonterminalIndex == symb.symbol.nonterminalIndex : symbol.terminal == symb.symbol.terminal);
    }
    bool operator!=(const Symbol &symb) const noexcept {
        return !(symbolType == symb.symbolType && 
            (symbolType ? symbol.nonterminalIndex == symb.symbol.nonterminalIndex : symbol.terminal == symb.symbol.terminal));
    }
    bool operator<(const Symbol &symb) const noexcept {
        return symbolType < symb.symbolType || 
            (symbolType ? symbol.nonterminalIndex < symb.symbol.nonterminalIndex : symbol.terminal < symb.symbol.terminal);
    }
    bool operator<=(const Symbol &symb) const noexcept {
        return symbolType <= symb.symbolType || 
            (symbolType ? symbol.nonterminalIndex <= symb.symbol.nonterminalIndex : symbol.terminal <= symb.symbol.terminal);
    }
    bool operator>(const Symbol &symb) const noexcept {
        return symbolType > symb.symbolType || 
            (symbolType ? symbol.nonterminalIndex > symb.symbol.nonterminalIndex : symbol.terminal > symb.symbol.terminal);
    }
    bool operator>=(const Symbol &symb) const noexcept {
        return symbolType >= symb.symbolType || 
            (symbolType ? symbol.nonterminalIndex >= symb.symbol.nonterminalIndex : symbol.terminal >= symb.symbol.terminal);
    }
};
inline void terminal(Symbol& symb, TokenType terminal) {symb.symbolType = 0; symb.symbol.terminal = terminal;}
inline void nonterminal(Symbol& symb, size_t nonterminalIndex) {symb.symbolType = 1; symb.symbol.nonterminalIndex = nonterminalIndex;}
#ifdef DEBUG
std::string strSubRule(const std::vector<Symbol>&, const std::vector<std::string>&);
std::string strrule(const std::vector<std::vector<Symbol>>&, const std::vector<std::string>&);
#endif

enum ErrorRecovery : unsigned char {POP, SCAN};

struct ParsingTableEntry {
    union {
        ErrorRecovery recoveryAction;
        size_t subRuleIndex;
        size_t &stackAction() noexcept {return subRuleIndex;} //type alias
    } action;
    unsigned actionType : 1; //1 or true value if stackAction
};

}

#include <functional>
template<> struct std::hash<SimpleSqlParser::Symbol> {
    size_t operator()(const SimpleSqlParser::Symbol &symb) const noexcept {
        return symb.symbolType ? symb.symbol.nonterminalIndex << 1 : ((size_t)symb.symbol.terminal << 1) + 1;
    }
};


#endif