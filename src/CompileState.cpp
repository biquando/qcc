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
        : os(os) {

    // Add builtin function signatures
    addFnDecl(new FnDeclNode(
        new TypeNode(BuiltinType::Void),
        "printi",
        std::vector<ParamNode *>{
            new ParamNode(new TypeNode(BuiltinType::Int), "i")
        })
    );
}

StackFrame *CompileState::getFrame(std::string identifier) {
    return frames.at(identifier);
}

void CompileState::addFrame(std::string identifier) {
    if (frames.find(identifier) != frames.end()) {
        std::cerr << "ERROR: Tried to add already-existing stack frame "
                  << identifier << '\n';
        exit(EXIT_FAILURE);
    }

    StackFrame *frame = new StackFrame(this);
    frames[identifier] = frame;
}

void CompileState::removeFrame(std::string identifier) {
    delete getFrame(identifier);
    frames.erase(identifier);
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
