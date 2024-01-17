#include "ast/ast.hpp"
#include "util.hpp"

TypeNode::TypeNode(BuiltinType builtinType)
        : kind(Builtin),
          builtinType(builtinType) {}

TypeNode::TypeNode(LiteralType literalType)
        : kind(Builtin),
          builtinType(toBuiltinType(literalType)) {}

TypeNode::TypeNode(std::string customType)
        : kind(Custom),
          customType(customType) {}

TypeNode::TypeNode(TypeNode *pointerType)
        : kind(Pointer),
          pointerType(pointerType) {}

unsigned TypeNode::size() {
    if (kind == Custom) {
        return 0;
    }

    if (kind == Pointer) {
        return 8;
    }

    switch (builtinType) {
        case BuiltinType::Int:
            return 8;
        case BuiltinType::Char:
            return 1;
        case BuiltinType::Void:
            return 0;
    }
}

bool TypeNode::validOp(BuiltinOperator op, TypeNode *otherType) {
    if (*this == TypeNode(BuiltinType::Void)) { return false; }

    if (kind == Pointer) {
        switch (op) {
            case BuiltinOperator::Plus:
            case BuiltinOperator::Minus:
                return otherType->kind != Pointer;
            case BuiltinOperator::Eq:
            case BuiltinOperator::Ne:
            case BuiltinOperator::Lt:
            case BuiltinOperator::Gt:
            case BuiltinOperator::Le:
            case BuiltinOperator::Ge:
                return otherType->kind == Pointer;
            default:
                return false;
        }
    }
    return true;
}

bool TypeNode::validOp(BuiltinOperator op) {
    if (*this == TypeNode(BuiltinType::Void)) { return false; }

    if (kind == Pointer) {
        switch (op) {
            case BuiltinOperator::Star:
            case BuiltinOperator::BitAnd:
            case BuiltinOperator::Not:
                return true;
            default:
                return false;
        }
    } else {
        if (op == BuiltinOperator::Star) { return false; }
    }
    return true;
}

bool TypeNode::operator==(const TypeNode &other) const {
    if (kind != other.kind) { return false; }

    switch (kind) {
        case Builtin:
            return builtinType == other.builtinType;
        case Custom:
            return customType == other.customType;
        case Pointer:
            return *pointerType == *other.pointerType;
    }
}

bool TypeNode::operator!=(const TypeNode &other) const {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, TypeNode &node) {
    os << "TypeNode: ";
    switch (node.kind) {
        case TypeNode::Builtin:
            os << node.builtinType;
            break;
        case TypeNode::Custom:
            os << node.customType;
            break;
        case TypeNode::Pointer:
            os << "*(" << *node.pointerType << ')';
            break;
    }
    return os;
}
