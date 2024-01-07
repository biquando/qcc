#include "ast/ast.hpp"
#include "util.hpp"

FnDeclNode::FnDeclNode(TypeNode *returnType, std::string identifier)
        : returnType(returnType),
          identifier(identifier) {}

FnDeclNode::FnDeclNode(TypeNode *returnType,
                       std::string identifier,
                       std::vector<ParamNode *> paramList)
        : returnType(returnType),
          identifier(identifier),
          paramList(paramList) {}

bool FnDeclNode::operator==(FnDeclNode &other) {
    if (paramList.size() != other.paramList.size()) {
        return false;
    }

    for (int i = 0; i < paramList.size(); i++) {
        if (*paramList[i]->type != *other.paramList[i]->type) {
            return false;
        }
    }

    return *returnType == *other.returnType
        &&  identifier == other.identifier;
}

bool FnDeclNode::operator!=(FnDeclNode &other) {
    return !(*this == other);
}

std::ostream &operator<<(std::ostream &os, FnDeclNode &node) {
    IndentedStream ios(os);
    os << "FnDefNode: (";
    os << *(node.returnType) << ") " << node.identifier << '(';

    if (!node.paramList.empty()) { os << '\n'; }
    for (auto &paramNode : node.paramList) {
        ios << *paramNode << '\n';
    }
    return os << ')';
}
