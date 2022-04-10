#ifndef __ERROR__
#define __ERROR__

#include <exception>
#include <string>
#include "lexer.hpp"

namespace SimpleSqlParser {

const char *constructMessage(const Lexer::Location &loc, const char *prefix = nullptr);

class SyntaxError : public std::exception {
    const char *_what;
public:
    SyntaxError(const char *what = nullptr);
    SyntaxError(const std::string &what) : SyntaxError(what.c_str()) {}
    SyntaxError(const Lexer::Location &loc, const char *what = nullptr) : _what(constructMessage(loc, what)) {}
    SyntaxError(const Lexer::Location &loc, const std::string &what) : _what(constructMessage(loc, what.c_str())) {}
    SyntaxError(const SyntaxError &other) : SyntaxError(other.what()) {}
    SyntaxError(SyntaxError &&other) noexcept : _what(other._what) {other._what = nullptr;}
    virtual ~SyntaxError() noexcept {if(_what) {delete _what; _what = nullptr;}}

    const char *what() const noexcept {return _what;}

    SyntaxError &operator=(const SyntaxError &);
    SyntaxError &operator=(SyntaxError &&other) noexcept {_what = other._what;other._what = nullptr; return *this;}
};

}

#endif