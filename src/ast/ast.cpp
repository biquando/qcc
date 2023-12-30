#include "ast.hpp"
#include "util.hpp"

std::ostream &operator<<(std::ostream &os, BuiltinType &type) {
    switch (type) {
        case BuiltinType::Void: return os << "Void";
        case BuiltinType::Int:  return os << "Int";
        case BuiltinType::Char: return os << "Char";
    }
}

std::ostream &operator<<(std::ostream &os, BuiltinOperator &op) {
    switch (op) {
        case BuiltinOperator::Plus:   return os << "Plus";
        case BuiltinOperator::Minus:  return os << "Minus";
        case BuiltinOperator::Star:   return os << "Star";
        case BuiltinOperator::Fslash: return os << "Fslash";
        case BuiltinOperator::Eq:     return os << "Eq";
        case BuiltinOperator::Ne:     return os << "Ne";
        case BuiltinOperator::Lt:     return os << "Lt";
        case BuiltinOperator::Gt:     return os << "Gt";
        case BuiltinOperator::Le:     return os << "Le";
        case BuiltinOperator::Ge:     return os << "Ge";
        case BuiltinOperator::Not:    return os << "Not";
    }
}

std::ostream &operator<<(std::ostream &os, LiteralType &type) {
    switch (type) {
        case LiteralType::Int:  return os << "Int";
        case LiteralType::Char: return os << "Char";
    }
}
