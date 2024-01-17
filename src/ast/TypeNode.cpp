#include "ast/ast.hpp"
#include "util.hpp"

TypeNode::TypeNode(BuiltinType builtinType)
        : isCustom(false),
          builtinType(builtinType) {}

TypeNode::TypeNode(std::string customType)
        : isCustom(true),
          customType(customType) {}

TypeNode::TypeNode(LiteralType literalType)
        : isCustom(false),
          builtinType(toBuiltinType(literalType)) {}

unsigned TypeNode::size() {
    if (isCustom) {
        return 0;
    }

    switch (builtinType) {
        case BuiltinType::Int:
            return 8;
        case BuiltinType::Char:
            return 1;
        case BuiltinType::Void:
            return 0;
    }
}

bool TypeNode::operator==(const TypeNode &other) const {
    if (isCustom != other.isCustom) { return false; }
    return isCustom
        ? customType == other.customType
        : builtinType == other.builtinType;
}

bool TypeNode::operator!=(const TypeNode &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, TypeNode &node) {
    os << "TypeNode: ";
    if (node.isCustom) {
        os << node.customType;
    } else {
        os << node.builtinType;
    }
    return os;
}
