#include "ast/ast.hpp"
#include "util.hpp"

ExprNode::ExprNode(LiteralNode *literal)
        : kind(Literal),
          literal(literal),
          type(new TypeNode(literal->type)) {}

ExprNode::ExprNode(std::string identifier, TypeNode *type)
        : kind(Identifier),
          identifier(identifier),
          type(type) {}

ExprNode::ExprNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall),
          type(fnCall->fnDecl->returnType) {}

ExprNode::ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2)
        : kind(BinaryOp),
          builtinOperator(binaryOperator),
          opr1(opr1),
          opr2(opr2) {

    if (opr1->type->isCustom || opr2->type->isCustom) {
        std::cerr << "ERROR: Tried binary op on custom type\n";
        exit(EXIT_FAILURE);
    }

    if (opr1->type->builtinType == BuiltinType::Void
     || opr2->type->builtinType == BuiltinType::Void) {
        std::cerr << "ERROR: Tried to do binary op on Void type\n";
        exit(EXIT_FAILURE);
     }

    if (*opr1->type == *opr2->type) {
        type = opr1->type;
    } else {
        type = new TypeNode(BuiltinType::Int);
    }
}

ExprNode::ExprNode(BuiltinOperator unaryOperator, ExprNode *opr)
        : kind(UnaryOp),
          builtinOperator(unaryOperator),
          opr(opr) {
    if (opr->type->isCustom) {
        std::cerr << "ERROR: Tried unary op on custom type\n";
        exit(EXIT_FAILURE);
    }

    if (opr->type->builtinType == BuiltinType::Void) {
        std::cerr << "ERROR: Tried unary op on Void type\n";
        exit(EXIT_FAILURE);
    }

    type = opr->type;
}

bool ExprNode::containsFnCalls() {
    return kind == FnCall
        || kind == BinaryOp
            && (opr1->containsFnCalls() || opr2->containsFnCalls())
        || kind == UnaryOp
            && opr->containsFnCalls();
}

std::ostream &operator<<(std::ostream &os, ExprNode &node) {
    IndentedStream ios(os);
    os << "ExprNode (" << node.type << ')';
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
