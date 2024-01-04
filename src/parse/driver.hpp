#pragma once

#include <string>
#include <vector>
#include "qcc.y.hpp"
#include "ast/ast.hpp"

#define YY_DECL yy::parser::symbol_type yylex(Driver &drv)
YY_DECL;

class Driver {
public:
    std::string execName;
    bool traceParsing;
    bool traceScanning;
    yy::location location;
    std::string file;

    std::vector<FnDefNode *> fnDefNodes;
    int res = 0;

    Driver(std::string execName);
    int parse(const std::string &f);
    void scan_begin();
    void scan_end();
};
