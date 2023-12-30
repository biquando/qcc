#include "ast.hpp"
#include "util.hpp"

LiteralNode::LiteralNode(int i)
        : type(LiteralType::Int),
          i(i) {}

LiteralNode::LiteralNode(char c)
        : type(LiteralType::Char),
          c(c) {}

std::ostream &operator<<(std::ostream &os, LiteralNode &node) {
    os << "LiteralNode ";
    switch (node.type) {
        case LiteralType::Int:
            os << node.i;
            break;
        case LiteralType::Char:
            os << '\'' << node.c << '\'';
            break;
    }
    return os;
}
