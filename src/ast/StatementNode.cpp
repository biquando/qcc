#include "ast/ast.hpp"
#include "util.hpp"

StatementNode::StatementNode(TypeNode *type, std::string identifier)
        : kind(Declaration),
          type(type),
          identifier(identifier) {}

StatementNode::StatementNode(TypeNode *type, std::string identifier, ExprNode *expr)
        : kind(Initialization),
          type(type),
          identifier(identifier),
          expr(expr) {}

StatementNode::StatementNode(std::string identifier, ExprNode *rexpr)
        : kind(Assignment),
          identifier(identifier),
          expr(rexpr) {}

StatementNode::StatementNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall) {}

std::ostream &operator<<(std::ostream &os, StatementNode &node) {
    IndentedStream ios(os);
    os << "StatementNode (";
    switch (node.kind) {
        case StatementNode::Declaration:
            os << "Declaration): (";
            os << *(node.type) << ") " << node.identifier;
            break;
        case StatementNode::Initialization:
            os << "Initialization): (";
            os << *(node.type) << ") " << node.identifier;
            ios << '\n' << *(node.expr);
            break;
        case StatementNode::Assignment:
            os << "Assignment): " << node.identifier;
            ios << '\n' << *(node.expr);
            break;
        case StatementNode::FnCall:
            os << "FnCall):";
            ios << '\n' << *(node.fnCall);
            break;
    }
    return os;
}
