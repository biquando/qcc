#include <cstdlib>
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
        : os(os),
          varTypes(new std::unordered_map<std::string, TypeNode *>()) {}

void CompileState::pushFrame() {
    frames.emplace_back();
}

StackFrame *CompileState::getTopFrame() {
    return &frames.back();
}

void CompileState::popFrame() {
    frames.pop_back();
}

TypeNode *CompileState::getVarType(std::string identifier) {
    if (varTypes->find(identifier) == varTypes->end()) {
        std::cerr << "ERROR: Couldn't find the type of " << identifier << '\n';
        exit(EXIT_FAILURE);
    }
    return (*varTypes)[identifier];
}

void CompileState::setVarType(std::string identifier, TypeNode *type) {
    if (varTypes->find(identifier) != varTypes->end()) {
        std::cerr << "ERROR: Tried to set type of alread-defined variable "
                  << identifier << '\n';
        exit(EXIT_FAILURE);
    }
    (*varTypes)[identifier] = type;
}
