#include <iostream>
#include <string>
#include "CompileState.hpp"

std::string toStr(Register res) {
    return "x" + std::to_string((int)res);
}

std::string toStr(long l) {
    return std::to_string(l);
}

CompileState::CompileState(std::ostream &os)
        : os(os) {}

void CompileState::pushFrame() {
    frames.emplace_back();
}

StackFrame *CompileState::getTopFrame() {
    return &frames.back();
}

void CompileState::popFrame() {
    frames.pop_back();
}
