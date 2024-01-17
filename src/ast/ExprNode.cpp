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
          type(fnCall->fnDecl->returnType) {
    if (*type == TypeNode(BuiltinType::Void)) {
        std::cerr << "ERROR: Can't use void function in expression\n";
        exit(EXIT_FAILURE);
    }
}

ExprNode::ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2)
        : kind(BinaryOp),
          builtinOperator(binaryOperator),
          opr1(opr1),
          opr2(opr2) {

    if (!opr1->type->validOp(binaryOperator, opr2->type)
     || !opr2->type->validOp(binaryOperator, opr1->type)) {
        std::cerr << "ERROR: Can't do binary op " << binaryOperator
                  << " on types (" << *(opr1->type) << ") and ("
                  << *(opr2->type) << ")\n";
        exit(EXIT_FAILURE);
    }

    if (opr1->type->kind == TypeNode::Pointer) {
        type = opr1->type;
        if (opr2->type->kind != TypeNode::Pointer) {
            this->opr2 = new ExprNode(
                BuiltinOperator::Star,
                opr2,
                new ExprNode(
                    new LiteralNode((long)opr1->type->pointerType->size())
                )
            );
        }
        return;
    }
    if (opr2->type->kind == TypeNode::Pointer) {
        type = opr2->type;
        if (opr1->type->kind != TypeNode::Pointer) {
            this->opr1 = new ExprNode(
                BuiltinOperator::Star,
                opr1,
                new ExprNode(
                    new LiteralNode((long)opr2->type->pointerType->size())
                )
            );
        }
        return;
    }

    if (*opr1->type == *opr2->type) {
        type = opr1->type;
        return;
    }

    type = new TypeNode(BuiltinType::Int);
}

ExprNode::ExprNode(BuiltinOperator unaryOperator, ExprNode *opr)
        : kind(UnaryOp),
          builtinOperator(unaryOperator),
          opr(opr) {

    if (!opr->type->validOp(unaryOperator)) {
        std::cerr << "ERROR: Can't do unary op " << unaryOperator
                  << " on type (" << *(opr1->type) << ")\n";
        exit(EXIT_FAILURE);
    }

    if (unaryOperator == BuiltinOperator::Star) {
        type = opr->type->pointerType;
        return;
    }

    if (unaryOperator == BuiltinOperator::BitAnd) {
        type = new TypeNode(opr->type);
        return;
    }

    type = opr->type;
}

ExprNode::ExprNode(std::vector<ExprNode *> *array)
        : kind(Array),
          array(array),
          type(new TypeNode(BuiltinType::Int)) {}

ExprNode::ExprNode()
        : kind(Empty),
          type(new TypeNode(BuiltinType::Void)) {}

bool ExprNode::containsFnCalls() {
    return kind == FnCall
        || kind == BinaryOp
            && (opr1->containsFnCalls() || opr2->containsFnCalls())
        || kind == UnaryOp
            && opr->containsFnCalls();
}

std::ostream &operator<<(std::ostream &os, ExprNode &node) {
    IndentedStream ios(os);
    os << "ExprNode (" << *(node.type) << ')';
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
        case ExprNode::Array:
            os << "(Array) {";
            if (!node.array->empty()) { os << '\n'; }
            for (ExprNode *elem : *node.array) {
                ios << *elem << '\n';
            }
            os << '}';
            break;
        case ExprNode::Empty:
            break;
    }
    return os;
}
