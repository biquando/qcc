#pragma once

#include <vector>
#include "qcc.y.hpp"
#include "ast.hpp"

#define YY_DECL yy::parser::symbol_type yylex(Driver &drv)
YY_DECL;

class Driver {
public:
    std::vector<FnDefNode *> fnDefNodes;

    Driver();
    int parse();
    void scan_begin();
    void scan_end();
};
