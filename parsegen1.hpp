//Parser Generator Phase 1
#ifndef __PARSEGEN1__
#define __PARSEGEN1__

#include <initializer_list>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include "parser.hpp"
#ifdef DEBUG 
#include <iostream>
#endif

namespace SimpleSqlParser {

//Factory class for Parser; phase 1 (rules may change)
class ParserGeneratorPhase1 {
#ifdef DEBUG 
public: //The following becomes implicitly public.
#endif
    //Master nonterminal array, of which is NOT SORTED
    std::vector<std::string> nonterminalArray;
    
    //This also needs to be hashable.
    struct IntermediateSymbol {
        union {
            TokenType terminal;
            std::string *nonterminal;
        } symbol;
        unsigned symbolType : 1; //1 or true value if nonterminal

        IntermediateSymbol(TokenType ttype = NONE) noexcept : symbolType(0) {symbol.terminal = ttype;}
        IntermediateSymbol(const std::string &nt) : symbolType(1) {symbol.nonterminal = new std::string(nt);}
        IntermediateSymbol(const char *nt) : symbolType(1) {symbol.nonterminal = new std::string(nt);}
        IntermediateSymbol(const IntermediateSymbol&);
        IntermediateSymbol(IntermediateSymbol&&) noexcept;
        ~IntermediateSymbol() noexcept;

        IntermediateSymbol& operator=(const IntermediateSymbol&);
        IntermediateSymbol& operator=(const std::string&);
        IntermediateSymbol& operator=(TokenType) noexcept;
        IntermediateSymbol& operator=(IntermediateSymbol&&) noexcept;
        
        bool operator==(const IntermediateSymbol &symb) const noexcept {
            return symbolType == symb.symbolType &&
                (symbolType ? *symbol.nonterminal == *symb.symbol.nonterminal : symbol.terminal == symb.symbol.terminal);
        }
        bool operator!=(const IntermediateSymbol &symb) const noexcept {
            return !(symbolType == symb.symbolType &&
                (symbolType ? *symbol.nonterminal == *symb.symbol.nonterminal : symbol.terminal == symb.symbol.terminal));
        }
        bool operator==(const std::string &nonterminal) const noexcept {return symbolType && *symbol.nonterminal == nonterminal;}
        bool operator==(TokenType ttype) const noexcept {return !symbolType && symbol.terminal == ttype;}

        bool operator<(const IntermediateSymbol &symb) const noexcept {
            return symbolType < symb.symbolType ||
                (symbolType ? *symbol.nonterminal < *symb.symbol.nonterminal : symbol.terminal < symb.symbol.terminal);
        }
        bool operator<=(const IntermediateSymbol &symb) const noexcept {
            return symbolType <= symb.symbolType ||
                (symbolType ? *symbol.nonterminal <= *symb.symbol.nonterminal : symbol.terminal <= symb.symbol.terminal);
        }
        bool operator>(const IntermediateSymbol &symb) const noexcept {
            return symbolType > symb.symbolType ||
                (symbolType ? *symbol.nonterminal > *symb.symbol.nonterminal : symbol.terminal > symb.symbol.terminal);
        }
        bool operator>=(const IntermediateSymbol &symb) const noexcept {
            return symbolType >= symb.symbolType ||
                (symbolType ? *symbol.nonterminal >= *symb.symbol.nonterminal : symbol.terminal >= symb.symbol.terminal);
        }
    };
#ifdef DEBUG 
    void outputRules();
#endif

    struct IntermediateSymbolHash {
        static std::hash<std::string> hashObject;
        constexpr size_t operator()(const IntermediateSymbol &symb) const noexcept {
            return symb.symbolType ? 
                hashObject(*(symb.symbol.nonterminal)) << 1 : (((size_t)symb.symbol.terminal) << 1) + 1;
        }
    };

    std::vector<std::vector<std::vector<IntermediateSymbol>>> rules;

    std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>> removeLeftRecursion(size_t);
    void removeLeftRecursion();
    std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>> leftFactoring(size_t);
    void leftFactoring();

    ParserGeneratorPhase1(); //Call default SQL CFG configured in cfg.cpp
    ParserGeneratorPhase1(std::initializer_list<std::pair<std::string, std::vector<std::vector<IntermediateSymbol>>>>);
};

}

#endif