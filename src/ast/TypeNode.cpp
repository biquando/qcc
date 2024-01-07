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

bool TypeNode::operator==(TypeNode &other) {
    if (isCustom != other.isCustom) { return false; }
    return isCustom
        ? customType == other.customType
        : builtinType == other.builtinType;
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
