#pragma once

#include <iostream>

class CompileState {
public:
    std::ostream &os;
    unsigned indent = 8;
    CompileState(std::ostream &os);
};
