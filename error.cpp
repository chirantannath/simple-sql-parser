#include <cstring>
#include <sstream>
#include "error.hpp"

namespace SimpleSqlParser {

const char *constructMessage(const Lexer::Location &loc, const char *prefix) {
    std::stringstream buffer(std::ios::out);
    if(prefix) buffer<<prefix;
    if(loc.lineNumber > 0) {
        buffer<<std::endl<<"at line "<<loc.lineNumber;
        if(loc.startColumnNumber > 0 && loc.endColumnNumber > 0)
            buffer<<", between columns "<<loc.startColumnNumber<<" & "<<loc.endColumnNumber;
        else if(loc.startColumnNumber > 0)
            buffer<<", at or near column "<<loc.startColumnNumber;
    }
    char * const target = new char[buffer.str().length()+1];
    std::strcpy(target, buffer.str().c_str());
    return const_cast<const char *>(target);
}

//SyntaxError
SyntaxError::SyntaxError(const char *what) {
    if(!what) {_what = nullptr; return;}
    char *target = new char[std::strlen(what)+1]; std::strcpy(target, what);
    _what = const_cast<const char *>(target);
}
SyntaxError& SyntaxError::operator=(const SyntaxError &other) {
    if(_what) delete _what;
    const char *what = other.what();
    if(!what) {_what = nullptr; return *this;}
    char *target = new char[std::strlen(what)+1]; std::strcpy(target, what);
    _what = const_cast<const char *>(target);
    return *this;
}

}