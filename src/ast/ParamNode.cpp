#include "ast.hpp"
#include "util.hpp"

ParamNode::ParamNode(TypeNode *type, std::string identifier)
        : type(type),
          identifier(identifier) {}

std::ostream &operator<<(std::ostream &os, ParamNode &node) {
    os << "ParamNode: (";
    os << *(node.type) << ") " << node.identifier;
    return os;
}
