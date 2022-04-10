#include "parsegen1.hpp"

namespace SimpleSqlParser {
//We have put the CFG for SQL in a separate file (this).
ParserGeneratorPhase1::ParserGeneratorPhase1() :
    ParserGeneratorPhase1({
        {"stmt_list", {
            {"stmt", EOSOP, "stmt_list"},
            {} //stmt_list can be empty
        }},
        {"stmt", {
            {}, //Empty
            {"insert_stmt"}
        }},
        {"insert_stmt", {
            {INSERT, INTO, IDENTIFIER, VALUES, PARENOPENOP, "constant_list", PARENCLOSEOP}
        }},
        {"constant_list", {
            {}, //Empty
            {"constant", COMMAOP, "constant_list"}
        }},
        {"constant", {
            {INT_CONSTANT}, {CHAR_CONSTANT}, {NUMBER_CONSTANT}
        }}
    })
{}

}