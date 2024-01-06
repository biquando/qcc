#include "ast/ast.hpp"
#include "util.hpp"

ExprNode::ExprNode(LiteralNode *literal)
        : kind(Literal),
          literal(literal) {}

ExprNode::ExprNode(std::string identifier)
        : kind(Identifier),
          identifier(identifier) {}

ExprNode::ExprNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall) {}

ExprNode::ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2)
        : kind(BinaryOp),
          builtinOperator(binaryOperator),
          opr1(opr1),
          opr2(opr2) {}

ExprNode::ExprNode(BuiltinOperator unaryOperator, ExprNode *opr)
        : kind(UnaryOp),
          builtinOperator(unaryOperator),
          opr(opr) {}

bool ExprNode::containsFnCalls() {
    return kind == FnCall
        || kind == BinaryOp
            && (opr1->containsFnCalls() || opr2->containsFnCalls())
        || kind == UnaryOp
            && opr->containsFnCalls();
}

std::ostream &operator<<(std::ostream &os, ExprNode &node) {
    IndentedStream ios(os);
    os << "ExprNode";
    switch (node.kind) {
        case ExprNode::Literal:
            os << " (" << *(node.literal) << ')';
            break;
        case ExprNode::Identifier:
            os << " (Identifier: " << node.identifier << ')';
            break;
        case ExprNode::FnCall:
            os << ": " << *(node.fnCall);
            break;
        case ExprNode::BinaryOp:
            os << " (BinaryOp: " << node.builtinOperator << ")\n";
            ios << *(node.opr1) << '\n';
            ios << *(node.opr2);
            break;
        case ExprNode::UnaryOp:
            os << " (UnaryOp: " << node.builtinOperator << ")\n";
            ios << *(node.opr);
            break;
    }
    return os;
}
