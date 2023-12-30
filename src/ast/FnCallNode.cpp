#include "ast.hpp"
#include "util.hpp"

FnCallNode::FnCallNode(ExprNode *expr, std::vector<ExprNode *> argList)
        : expr(expr), argList(argList) {}

FnCallNode::FnCallNode(ExprNode *expr)
        : expr(expr) {}

std::ostream &operator<<(std::ostream &os, FnCallNode &node) {
    IndentedStream ios(os);
    os << "FnCallNode: (" << *(node.expr) << ") (";
    if (!node.argList.empty()) { os << '\n'; }
    for (auto &arg : node.argList) {
        ios << *arg << '\n';
    }
    return os << ')';
}
