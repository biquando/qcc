#include <iostream>
#include <string>
#include "CompileState.hpp"

std::string toStr(Register res) {
    return "x" + std::to_string((int)res);
}

CompileState::CompileState(std::ostream &os)
        : os(os) {}

void CompileState::pushFrame(long initialStackPos = 0) {
    frames.emplace_back(initialStackPos);
}

StackFrame *CompileState::getTopFrame() {
    return &frames.back();
}

void CompileState::popFrame() {
    frames.pop_back();
}
