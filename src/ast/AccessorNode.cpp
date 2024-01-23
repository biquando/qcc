#include "ast/ast.hpp"
#include "util.hpp"

AccessorNode::AccessorNode(std::string identifier, TypeNode *type)
        : kind(Identifier),
          identifier(identifier),
          type(type) {}

AccessorNode::AccessorNode(ExprNode *expr)
        : kind(Dereference),
          expr(expr) {
    if (expr->type->kind != TypeNode::Pointer) {
        std::cerr << "ERROR: Can't dereference non-pointer type ("
                  << *(expr->type) << ")\n";
        exit(EXIT_FAILURE);
    }
    type = expr->type->pointerType;
}

std::ostream &operator<<(std::ostream &os, AccessorNode &node) {
    IndentedStream ios(os);
    os << "AccessorNode (";
    switch (node.kind) {
        case AccessorNode::Identifier:
            os << "Identifier): " << node.identifier;
            break;
        case AccessorNode::Dereference:
            os << "Dereference):\n";
            ios << *(node.expr);
            break;
    }
    return os;
}
