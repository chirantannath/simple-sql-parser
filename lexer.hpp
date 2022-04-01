#ifndef __LEXER__
#define __LEXER__
#include <istream>
#include <string>
#include <vector>
#include <string>
#include <stack>
#include <unordered_set>
#include <initializer_list>

namespace SimpleSqlParser {
typedef unsigned char ttype_parent;
typedef size_t mstate;

enum TokenType : ttype_parent {
    //Maintain priority order of tokens.
    NONE, 
    CREATE, TABLE, SELECT, INSERT, VALUES, INTO, PRIMARY, KEY, FROM, WHERE, BETWEEN, LIKE, IN, //Keywords
    STAROP, EQUALOP, GREATEROP, LESSOP, PARENOPENOP, PARENCLOSEOP, COMMAOP, EOSOP, //EOS=';' //Separator symbols
    INT, CHAR, NUMBER, //CHAR = VARCHAR; implies a string. NUMBER is double (IEE-754 double-precision floating point).
    INT_CONSTANT, CHAR_CONSTANT, NUMBER_CONSTANT, 
    //Keep INT_CONSTANT BEFORE NUMBER_CONSTANT because INT_CONSTANT is a subset of NUMBER_CONSTANT.
    IDENTIFIER, 
    EOI//Indicate end of all input.
};

class DFA {
protected:
    std::unordered_set<mstate> acceptStates;
    mstate startState, currentState;
    bool failed; //On any other error
    virtual mstate transition(mstate fromState, char ch) = 0;
public:
    const TokenType ttype;
    DFA(TokenType ttype = NONE) : failed(false), ttype(ttype) {}
    DFA(
        std::initializer_list<decltype(acceptStates)::value_type> acceptStates,
        mstate startState,
        TokenType ttype = NONE
    ) : acceptStates(acceptStates), startState(startState), currentState(startState), failed(false), ttype(ttype) {}
    virtual ~DFA() = default;

    void reset() {currentState = startState; failed = false;}
    virtual bool isAlphabet(char ch) const = 0;
    void process(char ch);
    void process(const char * const);
    bool isAccepting() const {return !failed && acceptStates.find(currentState) != acceptStates.end();}
};

struct Lexer {
    struct Location {size_t lineNumber, startColumnNumber, endColumnNumber;};
private:
    TokenType currentToken;
    std::string currentLexeme;
    std::istream *src;
    
    std::stack<char> pushback_buffer;
    Location location;
    bool isGood() const {return !pushback_buffer.empty() || src->good();}
    void pushback(char ch);

    const std::vector<DFA *> machines;
    static std::vector<DFA *> constructDFA();

    TokenType getNextToken();
public:
    Lexer(std::istream *src);
    virtual ~Lexer();

    TokenType getCurrentToken() const {return currentToken;}
    const char *getCurrentLexeme() const {return currentLexeme.c_str();}
    size_t getCurrentLexemeLength() const {return currentLexeme.length();}
};
}

#endif