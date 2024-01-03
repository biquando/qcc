#include "ast/ast.hpp"
#include "util.hpp"

TypeNode::TypeNode(BuiltinType builtinType)
        : builtinType(builtinType), isCustom(false) {}

TypeNode::TypeNode(std::string customType)
        : customType(customType), isCustom(true) {}

std::ostream &operator<<(std::ostream &os, TypeNode &node) {
    os << "TypeNode: ";
    if (node.isCustom) {
        os << node.customType;
    } else {
        os << node.builtinType;
    }
    return os;
}
