#include "parsegen3.hpp"
#include "parser.hpp"
#include "error.hpp"
#include <utility>
#ifdef DEBUG 
#include <iostream>
#endif

namespace SimpleSqlParser {
//Symbol
int Symbol::compare(const Symbol &symb) const noexcept {
    if(symbolType < symb.symbolType) return -1;
    else if(symbolType > symb.symbolType) return 1;
    else if(symbolType) {
        if(symbol.nonterminalIndex < symb.symbol.nonterminalIndex) return -1;
        else if(symbol.nonterminalIndex > symb.symbol.nonterminalIndex) return 1;
        else return 0;
    } else {
        if(symbol.terminal < symb.symbol.terminal) return -1;
        else if(symbol.terminal > symb.symbol.terminal) return 1;
        else return 0;
    }
}

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
std::string strStack(const std::deque<Symbol> &stack, const std::vector<std::string> &nonterminalArray) {
    std::string buffer("{"); size_t i = 0;
    for(Symbol symb : stack) {
        buffer += (symb.symbolType ? nonterminalArray[symb.symbol.nonterminalIndex] : TokenTypeNames[symb.symbol.terminal]);
        if(i < (stack.size() - 1)) buffer.push_back(' ');
        i++;
    } buffer.push_back('}');
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
const char *ErrorRecoveryNames[] = {"POP", "SCAN"}; 
#endif

Parser::Parser(ParserGeneratorPhase3 &pgp3, std::istream *src) : lexer(src), firstParse(false), unrecoverable(false) {init(pgp3);}
Parser::Parser(std::istream *src) : lexer(src), firstParse(false), unrecoverable(false) {ParserGeneratorPhase3 pgp3; init(pgp3);}
Parser::Parser() : firstParse(false), unrecoverable(false) {ParserGeneratorPhase3 pgp3; init(pgp3);}
void Parser::reopen(std::istream *src) {
    parsingStack.clear();
    lexer.reopen(src);
    firstParse = false; unrecoverable = false;
}
void Parser::init(ParserGeneratorPhase3 &pgp3) {
    pgp3.generateParsingTable();
#ifdef DEBUG 
    nonterminalArray = pgp3.nonterminalArray;
    rules = pgp3.rules; parsingTable = pgp3.parsingTable;
#else
    rules = std::move(pgp3.rules); parsingTable = std::move(pgp3.parsingTable);
#endif
}
void Parser::continueParse() {
    if(!firstParse) {
        parsingStack.insert(parsingStack.end(), {terminal(EOI), nonterminal(0)});
        lexer.getNextToken(); //get first lookahead token
        firstParse = true;
    }
    while(!parsingStack.empty()) {
#ifdef DEBUG 
        std::cout<<"\nStack is "<<strStack(parsingStack, nonterminalArray);
        std::cout<<"\nCurrent input terminal is: "; lexer.showstatus();
        std::cout<<"\nCurrent position: "<<constructMessageStr(lexer.getCurrentLexemeLocation())<<"\n\n";
#endif
        Symbol symbol(parsingStack.back()); //I need only to peek
        if(!symbol.symbolType && lexer.match(symbol.symbol.terminal)) {//We have a direct match
#ifdef DEBUG
            std::cout<<"We have a direct match. Going over to next input terminal.\n";
#endif
            parsingStack.pop_back(); continue;
        }
        else if(!symbol.symbolType) { //Unexpected token! Unrecoverable error.
            SyntaxError ex(lexer.getCurrentLexemeLocation(), 
#ifdef DEBUG                 
                std::string("Unrecoverable ") +
#endif
                std::string("Error; expected ") + TokenTypeNames[symbol.symbol.terminal] 
                + "; found " + lexer.getCurrentLexeme() + " [" + TokenTypeNames[lexer.getCurrentToken()] + "]");
            unrecoverable = (lexer.getCurrentToken() == EOI) ? true : false; //Try skipping over tokens I guess.
            if(!unrecoverable) lexer.getNextToken();
            throw ex;
        }
        const auto& action = parsingTable[symbol.symbol.nonterminalIndex][lexer.getCurrentToken()];
        if(!action.actionType) {//Error recovery
            SyntaxError ex(lexer.getCurrentLexemeLocation(), 
                std::string("Error; unexpected token ") 
                    + lexer.getCurrentLexeme() + " [" + TokenTypeNames[lexer.getCurrentToken()] + "]"
#ifdef DEBUG
                    + "; doing " + ErrorRecoveryNames[action.action.recoveryAction] + " to recover"
#endif
                    );
            switch(action.action.recoveryAction) {
            case POP: parsingStack.pop_back(); break;
            case SCAN: lexer.getNextToken(); break;
            }
            throw ex;
        }
        parsingStack.pop_back();
        //Directly push the rule on stack. Symbol matching should take care of the rest.
        const auto& subRule = rules[symbol.symbol.nonterminalIndex][action.action.subRuleIndex];
#ifdef DEBUG
        std::cout<<"Doing "<<nonterminalArray[symbol.symbol.nonterminalIndex]
            <<" ::= "<<strSubRule(subRule, nonterminalArray)<<std::endl;
#endif
        parsingStack.insert(parsingStack.end(), subRule.rbegin(), subRule.rend());
    }
    //Success!
}

}
