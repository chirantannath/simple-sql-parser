#ifndef __PARSER__
#define __PARSER__

#include "lexer.hpp"
#include <vector>
#include <deque>
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

    int compare(const Symbol&) const noexcept;

    bool operator==(const Symbol &symb) const noexcept {return compare(symb) == 0;}
    bool operator!=(const Symbol &symb) const noexcept {return compare(symb) != 0;}
    bool operator<(const Symbol &symb) const noexcept {return compare(symb) < 0;}
    bool operator<=(const Symbol &symb) const noexcept {return compare(symb) <= 0;}
    bool operator>(const Symbol &symb) const noexcept {return compare(symb) > 0;}
    bool operator>=(const Symbol &symb) const noexcept {return compare(symb) >= 0;}
};
inline Symbol terminal(TokenType ttype) {Symbol s; s.symbolType = 0; s.symbol.terminal = ttype; return s;}
inline Symbol nonterminal(size_t ntindex) {Symbol s; s.symbolType = 1; s.symbol.nonterminalIndex = ntindex; return s;}
#ifdef DEBUG
std::string strSubRule(const std::vector<Symbol>&, const std::vector<std::string>&);
std::string strrule(const std::vector<std::vector<Symbol>>&, const std::vector<std::string>&);
#endif

enum ErrorRecovery : unsigned char {POP, SCAN};
#ifdef DEBUG 
extern const char *ErrorRecoveryNames[];
#endif

struct ParsingTableEntry {
    union {
        ErrorRecovery recoveryAction;
        size_t subRuleIndex;
        size_t &stackAction() noexcept {return subRuleIndex;} //type alias
        size_t stackAction() const noexcept {return subRuleIndex;} //type alias
    } action;
    unsigned actionType : 1; //1 or true value if stackAction
};
inline ParsingTableEntry errorRecovery(ErrorRecovery action) 
{ParsingTableEntry e; e.actionType = 0; e.action.recoveryAction = action; return e;}
inline ParsingTableEntry stackAction(size_t subRuleIndex) 
{ParsingTableEntry e; e.actionType = 1; e.action.subRuleIndex = subRuleIndex; return e;}

//Main parser. requires ParserGeneratorPhase3.
class ParserGeneratorPhase3;
class Parser {
#ifdef DEBUG 
public:
    std::vector<std::string> nonterminalArray;
#endif
    std::vector<std::vector<std::vector<Symbol>>> rules;
    std::vector<std::vector<ParsingTableEntry>> parsingTable;
    std::deque<Symbol> parsingStack;
    Lexer lexer;
    
    Parser(ParserGeneratorPhase3&, std::istream *);
    void init(ParserGeneratorPhase3&);
public:
    Parser(std::istream *);
    Parser(); //No file?
    void reopen(std::istream *);
    void continueParse(); //continue or start; throws exception on error and can be used to resume even after error.
    
    //The following must be at the end since these are bit-fields.
private:
    unsigned firstParse : 1;
public:
    unsigned unrecoverable : 1; //Flag set if we get an unrecoverable error (usually unexpected tokens.)
};

}

#include <functional>
template<> struct std::hash<SimpleSqlParser::Symbol> {
    size_t operator()(const SimpleSqlParser::Symbol &symb) const noexcept {
        return symb.symbolType ? symb.symbol.nonterminalIndex << 1 : ((size_t)symb.symbol.terminal << 1) + 1;
    }
};


#endif