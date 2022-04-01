#include <utility>
#include <cstring>
#include <cctype>
#include <map>
#include "lexer.hpp"
#include "error.hpp"

namespace SimpleSqlParser {
//DFAs
void DFA::process(char ch) {
    if(failed) return;
    else if(!isAlphabet(ch)) {failed = true; return;}
    currentState = transition(currentState, ch);
}
void DFA::process(const char * const s) {for(const char *p = s; *p; p++) process(*p);}


//DFA subclass for strings with exact matches. Not used for SQL.
class ExactMatch : public virtual DFA {
    const char * const key;
    const size_t keylen;
protected:
    mstate transition(mstate s, char ch) {
        if(s >= keylen) {failed = true; return s;}
        if(key[s] == ch) return s+1;
        failed = true; return s;
    }
public:
    ExactMatch(const char * const key, TokenType ttype = NONE) : DFA(ttype), key(key), keylen(std::strlen(key)) {
        startState = currentState = 0;
        acceptStates.insert({keylen});
    }
    bool isAlphabet(char ch) const {return std::strchr(key, ch) ? true : false;}  //check for nonzero ptr
};

//DFA subclass for strings matching with case ignored.
class IgnoreCaseMatch : public virtual DFA {
    const char * const key;
    const size_t keylen;
protected:
    mstate transition(mstate s, char ch) {
        if(s >= keylen) {failed = true; return s;}
        if(key[s] == std::tolower(ch) || key[s] == std::toupper(ch)) return s+1;
        failed = true; return s;
    }
public:
    IgnoreCaseMatch(const char * const key, TokenType ttype = NONE) : DFA(ttype), key(key), keylen(std::strlen(key)) {
        startState = currentState = 0;
        acceptStates.insert({keylen});
    }
    bool isAlphabet(char ch) const {return std::strchr(key, std::tolower(ch)) || std::strchr(key, std::toupper(ch)) ? true : false;}
};

//DFA subclass for IDENTIFIER. Regex=a(a|d)* where a=alpha,d=digit
struct Identifier : public virtual DFA {
    Identifier() : DFA({2}, 0, IDENTIFIER) {}
    bool isAlphabet(char ch) const {return std::isdigit(ch) || std::isalpha(ch);}
protected:
    mstate transition(mstate s, char ch) {
        switch(s) {
        case 0:
            if(std::isalpha(ch)) return 2;
            else return 1;
        case 1: failed = true; return 1; //We know we have failed.
        case 2: return 2;
        }
        return s;
    }
};

//DFA subclass for INT_CONSTANT. Regex=(+|-|?)dd* where d=digit,?=epsilon
struct IntConstant : public virtual DFA {
    IntConstant() : DFA({0}, 1, INT_CONSTANT) {}
    bool isAlphabet(char ch) const {return ch == '+' || ch == '-' || std::isdigit(ch);}
protected:
    mstate transition(mstate s, char ch) {
        switch(s) {
        case 0: case 3:
            if(std::isdigit(ch)) return 0;
            else return 2;
        case 1:
            if(ch == '-' || ch == '+') return 3;
            else return 0;
        case 2: failed = true; return 2;
        }
        return s;
    }
};

//DFA subclass for CHAR_CONSTANT. Regex='c*'|"c*" where c=any character except ' and "
struct CharConstant : public virtual DFA {
    CharConstant() : DFA({2}, 0, CHAR_CONSTANT) {}
    bool isAlphabet(char ch) const {return true;}
protected:
    mstate transition(mstate s, char ch) {
        switch(s) {
        case 0: //{1,4}
            switch(ch) {
            case '\"': return 3;
            case '\'': return 1;
            default: return 4;
            }
        case 1: //{2,3}
            switch(ch) {
            case '\"': return 4;
            case '\'': return 2;
            default: return 1;
            }
        case 2: //{7}
        case 4: failed = true; return 4; //{}. we know we have failed.
        case 3: //{5,6}
            switch(ch) {
            case '\"': return 2;
            case '\'': return 4;
            default: return 3;
            }
        }
        return s;
    }
};

//DFA subclass for NUMBER_CONSTANT. Regex=(s|?)dd*pdd*(e(s|?)dd*|?) where s=+/-, e=literal e/E, p=literal decimal point '.', d=digit, ?=epsilon
struct NumberConstant : public virtual DFA {
    NumberConstant() : DFA({7,8}, 0, NUMBER_CONSTANT) {}
    bool isAlphabet(char ch) const {return std::isdigit(ch) || ch == '+' || ch == '-' || ch == 'e' || ch == 'E' || ch == '.';}
protected:
    mstate transition(mstate s, char ch) {
        switch(s) {
        case 0: //{1,2}
            if(ch == '.' || ch == 'e' || ch == 'E') return 5;
            else if(std::isdigit(ch)) return 1;
            else return 2;
        case 1: //{3,4}
            if(ch == '.') return 6;
            else if(std::isdigit(ch)) return 1;
            else return 5;
        case 2: //{2}
            if(std::isdigit(ch)) return 1;
            else return 5;
        case 3: //{8,9}
            if(ch == '.' || ch == 'e' || ch == 'E') return 5;
            else if(std::isdigit(ch)) return 8;
            else return 4;
        case 4: ///{9}
            if(std::isdigit(ch)) return 8;
            else return 5;
        case 5: failed = true; return 5; //{} We know we have failed.
        case 6: //{5}
            if(std::isdigit(ch)) return 7;
            else return 5;
        case 7: //{11,6,7}
            if(ch == '.' || ch == '+' || ch == '-') return 5;
            else if(std::isdigit(ch)) return 7;
            else return 3;
        case 8: //{10,11}
            if(std::isdigit(ch)) return 7;
            else return 5;
        }
        return s;
    }
};

//Lexer
std::vector<DFA *> Lexer::constructDFA() {
    std::vector<DFA *> mvec;
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
Lexer::Lexer(std::istream *src) : src(src), machines(constructDFA()), currentToken(NONE) {}
Lexer::~Lexer() {for(DFA *ptr : machines) delete ptr;}

}