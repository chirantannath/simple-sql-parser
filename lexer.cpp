#include <string>
#include <utility>
#include <cstring>
#include "lexer.hpp"
#include "error.hpp"
#ifdef DEBUG 
#include <iostream>
#endif

namespace SimpleSqlParser {
const std::array<TokenType, EOI+1> TokenTypes = {
    NONE, //None indicates an invalid token. Also used to indicate epsilon (empty string) during parser generation.
    CREATE, TABLE, SELECT, INSERT, VALUES, INTO, PRIMARY, KEY, FROM, WHERE, BETWEEN, LIKE, IN, //Keywords
    STAROP, EQUALOP, GREATEROP, LESSOP, PARENOPENOP, PARENCLOSEOP, COMMAOP, EOSOP, //EOS=';' //Separator symbols
    INT, CHAR, NUMBER, //CHAR = VARCHAR; implies a string. NUMBER is double (IEE-754 double-precision floating point).
    INT_CONSTANT, CHAR_CONSTANT, NUMBER_CONSTANT, 
    //Keep INT_CONSTANT BEFORE NUMBER_CONSTANT because INT_CONSTANT is a subset of NUMBER_CONSTANT.
    IDENTIFIER, 
    EOI//Indicate end of all input.
};

#ifdef DEBUG 
const char *TokenTypeNames[] = {
    "NONE",
    "CREATE", "TABLE", "SELECT", "INSERT", "VALUES", "INTO", "PRIMARY", "KEY", "FROM", "WHERE", "BETWEEN", "LIKE", "IN",
    "STAROP", "EQUALOP", "GREATEROP", "LESSOP", "PARENOPENOP", "PARENCLOSEOP", "COMMAOP", "EOSOP",
    "INT", "CHAR", "NUMBER",
    "INT_CONSTANT", "CHAR_CONSTANT", "NUMBER_CONSTANT",
    "IDENTIFIER",
    "EOI"
};
#endif

//DFAs
void DFA::process(char ch) noexcept {
    if(failed || noStateError) return;
    else if(!isAlphabet(ch)) {failed = 1; noStateError = 1; return;}
    currentState = transition(currentState, ch);
}
void DFA::process(const char * const s) noexcept {for(const char *p = s; *p; p++) process(*p);}


//DFA subclass for strings with exact matches. Not used for SQL.
class ExactMatch : public virtual DFA {
    const char * const key;
    const size_t keylen;
protected:
    mstate transition(mstate s, char ch) noexcept {
        if(s >= keylen) {failed = 1; noStateError = 1; return s;}
        if(key[s] == ch) return s+1;
        failed = 1; return s;
    }
public:
    ExactMatch(const char * const key, TokenType ttype = NONE) : DFA(ttype), key(key), keylen(std::strlen(key)) {
        startState = currentState = 0;
        acceptStates.insert({keylen});
    }
    bool isAlphabet(char ch) const noexcept {return std::strchr(key, ch) ? true : false;}  //check for nonzero ptr
};

//DFA subclass for strings matching with case ignored.
class IgnoreCaseMatch : public virtual DFA {
    const char * const key;
    const size_t keylen;
protected:
    mstate transition(mstate s, char ch) noexcept {
        if(s >= keylen) {failed = 1; noStateError = 1; return s;}
        if(key[s] == std::tolower(ch) || key[s] == std::toupper(ch)) return s+1;
        failed = 1; return s;
    }
public:
    IgnoreCaseMatch(const char * const key, TokenType ttype = NONE) : DFA(ttype), key(key), keylen(std::strlen(key)) {
        startState = currentState = 0;
        acceptStates.insert({keylen});
    }
    bool isAlphabet(char ch) const noexcept {return std::strchr(key, std::tolower(ch)) || std::strchr(key, std::toupper(ch)) ? true : false;}
};

//DFA subclass for IDENTIFIER. Regex=a(a|d)* where a=alpha,d=digit
struct Identifier : public virtual DFA {
    Identifier() : DFA({2}, 0, IDENTIFIER) {}
    bool isAlphabet(char ch) const noexcept {return std::isdigit(ch) || std::isalpha(ch);}
protected:
    mstate transition(mstate s, char ch) noexcept {
        switch(s) {
        case 0:
            if(std::isalpha(ch)) return 2;
            else {failed = 1; return 1;}
        case 1: failed = 1; return 1; //We know we have failed.
        case 2: return 2;
        }
        noStateError = 1; return s;
    }
};

//DFA subclass for INT_CONSTANT. Regex=(+|-|?)dd* where d=digit,?=epsilon.
struct IntConstant : public virtual DFA {
    IntConstant() : DFA({0}, 1, INT_CONSTANT) {}
    bool isAlphabet(char ch) const noexcept {return ch == '+' || ch == '-' || std::isdigit(ch);}
protected:
    mstate transition(mstate s, char ch) noexcept {
        switch(s) {
        case 0: case 3:
            if(std::isdigit(ch)) return 0;
            else {failed = 1; return 2;}
        case 1:
            if(ch == '-' || ch == '+') return 3;
            else return 0;
        case 2: failed = 1; return 2;
        }
        noStateError = 1; return s;
    }
};

//DFA subclass for CHAR_CONSTANT. Regex='c*'|"c*" where c=any character except ' and "
struct CharConstant : public virtual DFA {
    CharConstant() : DFA({2}, 0, CHAR_CONSTANT) {}
    bool isAlphabet(char ch) const noexcept {return true;}
protected:
    mstate transition(mstate s, char ch) noexcept {
        switch(s) {
        case 0: //{1,4}
            switch(ch) {
            case '\"': return 3;
            case '\'': return 1;
            default: failed = 1; return 4;
            }
        case 1: //{2,3}
            switch(ch) {
            case '\"': failed = 1; return 4;
            case '\'': return 2;
            default: return 1;
            }
        case 2: //{7}
        case 4: failed = 1; return 4; //{}. we know we have failed.
        case 3: //{5,6}
            switch(ch) {
            case '\"': return 2;
            case '\'': failed = 1; return 4;
            default: return 3;
            }
        }
        noStateError = 1; return s;
    }
};

//DFA subclass for NUMBER_CONSTANT. Regex=(s|?)dd*pdd*(e(s|?)dd*|?) where s=+/-, e=literal e/E, p=literal decimal point '.', d=digit, ?=epsilon
struct NumberConstant : public virtual DFA {
    NumberConstant() : DFA({7,8}, 0, NUMBER_CONSTANT) {}
    bool isAlphabet(char ch) const noexcept {return std::isdigit(ch) || ch == '+' || ch == '-' || ch == 'e' || ch == 'E' || ch == '.';}
protected:
    mstate transition(mstate s, char ch) noexcept {
        switch(s) {
        case 0: //{1,2}
            if(ch == '.' || ch == 'e' || ch == 'E') {failed = 1; return 5;}
            else if(std::isdigit(ch)) return 1;
            else return 2;
        case 1: //{3,4}
            if(ch == '.') return 6;
            else if(std::isdigit(ch)) return 1;
            else {failed = 1; return 5;}
        case 2: //{2}
            if(std::isdigit(ch)) return 1;
            else return 5;
        case 3: //{8,9}
            if(ch == '.' || ch == 'e' || ch == 'E') {failed = 1; return 5;}
            else if(std::isdigit(ch)) return 8;
            else return 4;
        case 4: ///{9}
            if(std::isdigit(ch)) return 8;
            else {failed = 1; return 5;}
        case 5: failed = 1; return 5; //{} We know we have failed.
        case 6: //{5}
            if(std::isdigit(ch)) return 7;
            else {failed = 1; return 5;}
        case 7: //{11,6,7}
            if(ch == '.' || ch == '+' || ch == '-') {failed = 1; return 5;}
            else if(std::isdigit(ch)) return 7;
            else return 3;
        case 8: //{10,11}
            if(std::isdigit(ch)) return 7;
            else {failed = 1; return 5;}
        }
        noStateError = 1; return s;
    }
};

//Lexer
char Lexer::getChar() {
    char ch=0; //We get a null character when src->get(ch) does not change ch.
    if(pushback_buffer.empty()) src->get(ch);
    else {ch = pushback_buffer.top(); pushback_buffer.pop();}
    if(ch == '\n') {currentLineNumber++; currentColumnNumber = 1;}
    else currentColumnNumber++;
    return ch;
}
char Lexer::peekChar() {
    char ch=0; //We get a null character when src->get(ch) does not change ch.
    if(pushback_buffer.empty()) {
        src->get(ch);
        pushback_buffer.push(ch);
    } else ch = pushback_buffer.top();
    return ch;
}
void Lexer::pushback(char ch) {
    pushback_buffer.push(ch);
    if(ch == '\n') {currentLineNumber--; currentColumnNumber = 0;} //We do not know the column number in previous line.
    else currentColumnNumber--;
}
std::vector<DFA *> Lexer::constructDFA() {
    std::vector<DFA *> mvec; mvec.reserve(TokenTypes.size());
    mvec.push_back(new IgnoreCaseMatch("CREATE", CREATE));
    mvec.push_back(new IgnoreCaseMatch("TABLE", TABLE));
    mvec.push_back(new IgnoreCaseMatch("SELECT", SELECT));
    mvec.push_back(new IgnoreCaseMatch("INSERT", INSERT));
    mvec.push_back(new IgnoreCaseMatch("VALUES", VALUES));
    mvec.push_back(new IgnoreCaseMatch("INTO", INTO));
    mvec.push_back(new IgnoreCaseMatch("PRIMARY", PRIMARY));
    mvec.push_back(new IgnoreCaseMatch("KEY", KEY));
    mvec.push_back(new IgnoreCaseMatch("FROM", FROM));
    mvec.push_back(new IgnoreCaseMatch("WHERE", WHERE));
    mvec.push_back(new IgnoreCaseMatch("BETWEEN", BETWEEN));
    mvec.push_back(new IgnoreCaseMatch("LIKE", LIKE));
    mvec.push_back(new IgnoreCaseMatch("IN", IN));
    mvec.push_back(new IgnoreCaseMatch("*", STAROP));
    mvec.push_back(new IgnoreCaseMatch("=", EQUALOP));
    mvec.push_back(new IgnoreCaseMatch(">", GREATEROP));
    mvec.push_back(new IgnoreCaseMatch("<", LESSOP));
    mvec.push_back(new IgnoreCaseMatch("(", PARENOPENOP));
    mvec.push_back(new IgnoreCaseMatch(")", PARENCLOSEOP));
    mvec.push_back(new IgnoreCaseMatch(",", COMMAOP));
    mvec.push_back(new IgnoreCaseMatch(";", EOSOP));
    mvec.push_back(new IgnoreCaseMatch("INT", INT));
    mvec.push_back(new IgnoreCaseMatch("INTEGER", INT));
    mvec.push_back(new IgnoreCaseMatch("CHAR", CHAR));
    mvec.push_back(new IgnoreCaseMatch("VARCHAR", CHAR));
    mvec.push_back(new IgnoreCaseMatch("NUMBER", NUMBER));
    mvec.push_back(new IgnoreCaseMatch("NUMERIC", NUMBER));
    mvec.push_back(new IgnoreCaseMatch("FLOAT", NUMBER));
    mvec.push_back(new IgnoreCaseMatch("DOUBLE", NUMBER));
    mvec.push_back(new IntConstant);
    mvec.push_back(new CharConstant);
    mvec.push_back(new NumberConstant);
    mvec.push_back(new Identifier);
    mvec.shrink_to_fit(); return mvec;
}
TokenType Lexer::getNextToken() {
    DFA *targetMachine; //We have to eventually narrow it down to one machine.
    char ch; 
    currentToken = NONE; currentLexeme.clear();
    //If we are not good, we are EOI.
    if(!isGood()) {return currentToken = EOI;}
    ignoreWhitespaces(); //Skip over whitespace.
    currentLexemeLocation = {currentLineNumber, currentColumnNumber, 0}; //Track current location
    //If we are not good, we are EOI.
    if(!isGood()) {return currentToken = EOI;}
    //Reset all DFAs and push current to all DFAs in lockstep; and
    //find the machines which can still continue with the pushed char (noStateError is false). May be multiple.
    ch = getChar(); currentLexeme.push_back(ch); 
    std::vector<DFA *> currentMachines; currentMachines.reserve(machines.size());
    size_t currentLexemeAcceptLength = 0; //Length of the substring of currentLexeme (from start position) which forms any valid token.
    targetMachine = nullptr;
    for(DFA *machine : machines) {
        machine->reset(); 
        machine->process(ch); 
        if(machine->isAccepting() && !targetMachine) {
            currentLexemeAcceptLength = 1; targetMachine = machine; //Possible target. No guarantee.
            currentMachines.push_back(machine);
        }
        else if(!machine->isPermaFailed()) currentMachines.push_back(machine);
    }
    if(currentMachines.empty()) throw SyntaxError(currentLexemeLocation, "Unrecognized character: code=" + std::to_string(ch) + ", \'" + std::string(1, ch) + "\'");
    //Stores the machines which can still run with the character in the next iteration.
    std::vector<DFA *> nextCurrentMachines; 
    nextCurrentMachines.reserve(currentMachines.size());
    //Iterate over incoming chars to create lexeme.
    while(isGood() && !currentMachines.empty()) {
        nextCurrentMachines.clear();
        ch = peekChar();
        //Push to DFAs in lockstep.
        DFA *nextTargetMachine = nullptr;
        for(DFA *machine : currentMachines) {
            machine->process(ch);
            if(machine->isAccepting() && !nextTargetMachine) {
                nextTargetMachine = machine;
                currentLexemeAcceptLength = currentLexeme.length()+1;
                nextCurrentMachines.push_back(machine);
            }
            else if(!machine->isPermaFailed()) nextCurrentMachines.push_back(machine);
        }
        if(nextTargetMachine) targetMachine = nextTargetMachine; //Another possible target.
        currentLexeme.push_back(ch); getChar(); //advance
        //Maybe all have rejected. which means multiple have accepted the lexeme on some previous iteration.
        //We continue if some have still accepted the char (we display greedy behaviour)
        std::swap(currentMachines, nextCurrentMachines); //Specialization for std::vector exists.
    }
    //Loop breaks if currentMachines is empty or we have reached EOI. We may have found a possible target machine.
    if(!targetMachine || currentLexemeAcceptLength == 0) throw SyntaxError(currentLexemeLocation, "Unrecognized character sequence \"" + currentLexeme + "\"");
    //If targetMachine is found we need to push back the extra characters.
    while(currentLexeme.length() > currentLexemeAcceptLength) {
        pushback(currentLexeme.back()); currentLexeme.pop_back();
    }
    currentLexemeLocation.endColumnNumber = currentColumnNumber > 0 ? currentColumnNumber-1 : 0;
    return currentToken = targetMachine->ttype;
}
#ifdef DEBUG
void Lexer::showstatus() {
    std::cout<<"[LEXER: ttype="<<TokenTypeNames[currentToken]<<","<<currentLexeme<<","<<currentLineNumber
        <<","<<currentColumnNumber<<"]\n";
} 
#endif
Lexer::Lexer(std::istream *src) : src(src), machines(constructDFA()), currentToken(NONE), currentLineNumber(1),
    currentColumnNumber(1) {}
Lexer::~Lexer() noexcept {for(DFA *ptr : machines) delete ptr;}
bool Lexer::match(TokenType mttype)  {return false;}
}