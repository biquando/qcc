#include "ast/ast.hpp"
#include "util.hpp"

FnCallNode::FnCallNode(std::string identifier, TypeNode *retType, std::vector<ExprNode *> argList)
        : identifier(identifier), retType(retType), argList(argList) {}

FnCallNode::FnCallNode(std::string identifier, TypeNode *retType)
        : identifier(identifier), retType(retType) {}

std::ostream &operator<<(std::ostream &os, FnCallNode &node) {
    IndentedStream ios(os);
    os << "FnCallNode: " << node.identifier << "(";
    if (!node.argList.empty()) { os << '\n'; }
    for (auto &arg : node.argList) {
        ios << *arg << '\n';
    }
    return os << ')';
}
