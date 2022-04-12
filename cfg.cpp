#include "lexer.hpp"
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
            {"select_stmt"},
            {"insert_stmt"},
            {"create_table_stmt"},
        }},
        
        {"select_stmt", {
            {SELECT, "identifier_list", "from_stmt", "where_stmt"},
            {SELECT, STAROP, "from_stmt", "where_stmt"}
        }},
        {"from_stmt", {
            {},
            {FROM, "identifier_list"}
        }},
        {"where_stmt", {
            {},
            {WHERE, "condition_expr"}
        }},

        {"insert_stmt", {
            {INSERT, INTO, IDENTIFIER, VALUES, PARENOPENOP, "constant_list", PARENCLOSEOP},
            {INSERT, INTO, IDENTIFIER, PARENOPENOP, "identifier_list", PARENCLOSEOP, 
                VALUES, PARENOPENOP, "constant_list", PARENCLOSEOP}
        }},

        {"create_table_stmt", {
            //{CREATE, TABLE, IDENTIFIER, PARENOPENOP, PARENCLOSEOP},
            {CREATE, TABLE, IDENTIFIER, PARENOPENOP, "decl_list", PARENCLOSEOP}
        }},

        {"condition_expr", {
            {"condition_expr", OR, "condition_term"}, {"condition_term"}
        }},
        {"condition_term", {
            {"condition_term", AND, "condition_op"}, {"condition_op"}
        }},
        {"condition_op", {
            {NOT, PARENOPENOP, "condition_expr", PARENCLOSEOP},
            {PARENOPENOP, "condition_expr", PARENCLOSEOP},
            {"identifier_or_constant", LESSOP, "identifier_or_constant"},
            {"identifier_or_constant", GREATEROP, "identifier_or_constant"},
            {"identifier_or_constant", EQUALOP, "identifier_or_constant"},
            {"identifier_or_constant", BETWEEN, "identifier_or_constant", AND, "identifier_or_constant"},
            {"identifier_or_constant", LIKE, CHAR_CONSTANT},
            {"identifier_or_constant", IN, PARENOPENOP, "constant_list", PARENCLOSEOP},
            //No nesting of SELECT supported.
            //{"identifier_or_constant", IN, PARENOPENOP, "select_stmt", PARENCLOSEOP}
        }},
        
        {"decl_list", {
            {},
            {"var_decl"},
            {"var_decl", COMMAOP, "primary_key_decl"},
            {"var_decl", COMMAOP, "decl_list"},
        }},
        {"var_decl", {
            {IDENTIFIER, INT},
            {IDENTIFIER, INT, PARENOPENOP, INT_CONSTANT, PARENCLOSEOP},
            {IDENTIFIER, CHAR},
            {IDENTIFIER, CHAR, PARENOPENOP, INT_CONSTANT, PARENCLOSEOP},
            {IDENTIFIER, NUMBER},
            {IDENTIFIER, NUMBER, PARENOPENOP, INT_CONSTANT, PARENCLOSEOP},
            {IDENTIFIER, NUMBER, PARENOPENOP, INT_CONSTANT, COMMAOP, INT_CONSTANT, PARENCLOSEOP}
        }},
        {"primary_key_decl", {
            {PRIMARY, KEY, PARENOPENOP, "identifier_list", PARENCLOSEOP}
        }},

        {"identifier_list", {
            {},
            {IDENTIFIER, COMMAOP, "identifier_list"},
            {IDENTIFIER}
        }},
        {"constant_list", {
            {}, //Empty
            {"constant", COMMAOP, "constant_list"},
            {"constant"}
        }},
        {"identifier_or_constant", {
            {IDENTIFIER}, {"constant"}
        }},
        {"constant", {
            {INT_CONSTANT}, {CHAR_CONSTANT}, {NUMBER_CONSTANT}
        }},
    })
{}

}