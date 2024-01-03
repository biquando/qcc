#include "ast/ast.hpp"
#include "util.hpp"

FnCallNode::FnCallNode(std::string identifier, std::vector<ExprNode *> argList)
        : identifier(identifier), argList(argList) {}

FnCallNode::FnCallNode(std::string identifier)
        : identifier(identifier) {}

std::ostream &operator<<(std::ostream &os, FnCallNode &node) {
    IndentedStream ios(os);
    os << "FnCallNode: " << node.identifier << "(";
    if (!node.argList.empty()) { os << '\n'; }
    for (auto &arg : node.argList) {
        ios << *arg << '\n';
    }
    return os << ')';
}
