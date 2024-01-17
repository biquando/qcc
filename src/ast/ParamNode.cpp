#include "ast/ast.hpp"
#include "util.hpp"

ParamNode::ParamNode(TypeNode *type, std::string identifier)
        : type(type),
          identifier(identifier) {
    if (*type == TypeNode(BuiltinType::Void)) {
        std::cerr << "ERROR: Can't declare parameter with void type\n";
        exit(EXIT_FAILURE);
    }
}

std::ostream &operator<<(std::ostream &os, ParamNode &node) {
    os << "ParamNode: (";
    os << *(node.type) << ") " << node.identifier;
    return os;
}
