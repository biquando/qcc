#include <cstdlib>
#include <iostream>
#include <string>
#include "ast/ast.hpp"
#include "CompileState.hpp"
#include "util.hpp"

StaticData::StaticData(unsigned long id, std::string string)
        : kind(String),
          string(string),
          id(id) {
    ptrType = new TypeNode(new TypeNode(BuiltinType::Char));
}

StaticData::StaticData()
        : kind(None) {}

void StaticData::emit(CompileState &cs) {
    IndentedStream ios(cs.os, cs.indent);

    ios << ".data\n";

    unsigned align = p2alignment();
    if (align > 0) {
        ios << ".p2align " << p2alignment() << "\n";
    }

    cs.os << label() << ":\n";
    switch (kind) {
        case String:
            ios << ".asciz " << string << "\n";
            break;
        case None: break;
    }
}

unsigned StaticData::p2alignment() {
    switch (kind) {
        case String: return 0;
        case None: return 0;
    }
}

std::string StaticData::label() {
    std::string kindStr;
    switch (kind) {
        case String:
            kindStr = "String";
            break;
        case None: break;
    }
    return "static." + kindStr + "." + std::to_string(id);
}

std::ostream &operator<<(std::ostream &os, StaticData &staticData) {
    switch (staticData.kind) {
        case StaticData::String:
            os << staticData.string;
            break;
        case StaticData::None:
            break;
    }
    return os;
}
