#ifndef __LEXER__
#define __LEXER__
#include <istream>
#include <string>
#include <vector>
#include <deque>
#include <unordered_set>
#include <initializer_list>
#include <array>
#include <cctype>

namespace SimpleSqlParser {
typedef unsigned char ttype_parent;
typedef size_t mstate;

enum TokenType : ttype_parent {
    //Maintain priority order of tokens.
    NONE, //None indicates an invalid token. Also used to indicate epsilon (empty string) during parser generation.
    CREATE, TABLE, SELECT, INSERT, VALUES, INTO, PRIMARY, KEY, FROM, WHERE, BETWEEN, LIKE, IN, AND, OR, NOT, //Keywords
    STAROP, EQUALOP, GREATEROP, LESSOP, PARENOPENOP, PARENCLOSEOP, COMMAOP, EOSOP, //EOS=';' //Separator symbols
    INT, CHAR, NUMBER, //CHAR = VARCHAR; implies a string. NUMBER is double (IEE-754 double-precision floating point).
    INT_CONSTANT, CHAR_CONSTANT, NUMBER_CONSTANT, 
    //Keep INT_CONSTANT BEFORE NUMBER_CONSTANT because INT_CONSTANT is a subset of NUMBER_CONSTANT.
    IDENTIFIER, 
    EOI//Indicate end of all input.
};
extern const std::array<TokenType, EOI+1> TokenTypes;

extern const char *TokenTypeNames[];

class DFA {
protected:
    std::unordered_set<mstate> acceptStates;
    mstate startState, currentState;
    unsigned failed : 1; //On any other error
    unsigned noStateError : 1; 
    virtual mstate transition(mstate fromState, char ch) noexcept = 0;
public:
    const TokenType ttype;
    DFA(TokenType ttype = NONE) : failed(0), noStateError(0), ttype(ttype) {}
    DFA(
        std::initializer_list<decltype(acceptStates)::value_type> acceptStates,
        mstate startState,
        TokenType ttype = NONE
    ) : acceptStates(acceptStates), startState(startState), currentState(startState), failed(0), noStateError(0), ttype(ttype) {}
    virtual ~DFA() noexcept = default;

    void reset() noexcept {currentState = startState; failed = 0; noStateError = 0;}
    virtual bool isAlphabet(char ch) const noexcept = 0;
    void process(char ch) noexcept;
    void process(const char * const) noexcept;
    bool isAccepting() const noexcept {return !failed && !noStateError && acceptStates.find(currentState) != acceptStates.end();}
    bool isNoStateError() const noexcept {return noStateError ? true : false;}
    bool isPermaFailed() const noexcept {return failed || noStateError;}
};

struct Lexer {
    struct Location {size_t lineNumber, startColumnNumber, endColumnNumber;};
private:
    TokenType currentToken;
    std::string currentLexeme;
    Location currentLexemeLocation;
    std::istream *src;
    
    std::deque<char> pushback_buffer;
    decltype(Location::lineNumber) currentLineNumber;
    decltype(Location::startColumnNumber) currentColumnNumber;

    bool isGood() const noexcept {return !pushback_buffer.empty() || src->good();}
    char getChar(); char peekChar();
    void pushback(char); //buffer overflow may happen
    void ignoreWhitespaces();// {while(isGood() && (std::isspace(peekChar()) || peekChar()==0)) getChar();}

    const std::vector<DFA *> machines;
    static std::vector<DFA *> constructDFA();

public:
    TokenType getNextToken();
#ifdef DEBUG 
    void showstatus(); //Show current token status to standard output
#endif
    Lexer(std::istream *src);
    Lexer();
    virtual ~Lexer() noexcept;

    TokenType getCurrentToken() const noexcept {return currentToken;}
    const char *getCurrentLexeme() const noexcept {return currentLexeme.c_str();}
    const Location &getCurrentLexemeLocation() const noexcept {return currentLexemeLocation;}
    size_t getCurrentLexemeLength() const noexcept {return currentLexeme.length();}
    decltype(currentLineNumber) getCurrentLineNumber() const noexcept {return currentLineNumber;}
    decltype(currentColumnNumber) getCurrentColumnNumber() const noexcept {return currentColumnNumber;}

    bool match(TokenType);
    void reopen(std::istream *src);
};
}

#endif