#include <cstdlib>
#include <iostream>
#include <string>
#include "CompileState.hpp"

std::string toStr(Register res, std::string regPrefix) {
    return regPrefix + std::to_string((int)res);
}

std::string toStr(long l) {
    return std::to_string(l);
}

std::ostream &operator<<(std::ostream &os, Register &reg) {
    return os << 'x' << (int)reg;
}

CompileState::CompileState(std::ostream &os)
        : os(os),
          varTypes(new std::unordered_map<std::string, TypeNode *>()) {

    // Add builtin function signatures
    addFnDecl(new FnDeclNode(
        new TypeNode(BuiltinType::Void),
        "printi",
        std::vector<ParamNode *>{
            new ParamNode(new TypeNode(BuiltinType::Int), "i")
        })
    );

    // Not a technically function, but still has a signature
    addFnDecl(new FnDeclNode(
        new TypeNode(BuiltinType::Int),
        "svc",
        std::vector<ParamNode *>{
            new ParamNode(new TypeNode(BuiltinType::Int), "syscall")
        })
    );
}

void CompileState::pushFrame(FnDefNode *fnDef) {
    frames.emplace_back(this, fnDef);
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

FnDeclNode *CompileState::getFnDecl(std::string identifier) {
    if (fnDecls.find(identifier) != fnDecls.end()) {
        return fnDecls.at(identifier);
    }
    if (fnDefs.find(identifier) != fnDefs.end()) {
        return fnDefs.at(identifier);
    }
    std::cerr << "ERROR: Function " << identifier << " is not declared\n";
    exit(EXIT_FAILURE);
}

void CompileState::addFnDecl(FnDeclNode *fnDecl) {
    std::string &identifier = fnDecl->identifier;

    if (fnDecls.find(identifier) != fnDecls.end()
            && *fnDecl != *fnDecls.at(identifier)) {
        goto error_wrong_types;
    }

    if (fnDefs.find(identifier) != fnDefs.end()
            && *fnDecl != *fnDefs.at(identifier)) {
        goto error_wrong_types;
    }

    fnDecls[identifier] = fnDecl;
    return;

error_wrong_types:
    std::cerr << "ERROR: Tried to redeclare function " << identifier
              << " with different types\n";
    exit(EXIT_FAILURE);
}

void CompileState::addFnDef(FnDefNode *fnDef) {
    std::string &identifier = fnDef->identifier;

    if (fnDecls.find(identifier) != fnDecls.end()
            && *fnDef != *fnDecls.at(identifier)) {
        std::cerr << "ERROR: Tried to redeclare function " << identifier
                << " with different types\n";
        exit(EXIT_FAILURE);
    }

    if (fnDefs.find(identifier) != fnDefs.end()) {
        std::cerr << "ERROR: Tried to redefine function " << identifier << '\n';
        exit(EXIT_FAILURE);
    }

    fnDefs[identifier] = fnDef;
}
