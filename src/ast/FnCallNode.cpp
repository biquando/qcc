#include "ast/ast.hpp"
#include "util.hpp"

FnCallNode::FnCallNode(std::string identifier,
                       FnDeclNode *fnDecl,
                       std::vector<ExprNode *> argList)
        : identifier(identifier),
          fnDecl(fnDecl),
          argList(argList) {
    verifyParameters();
}

FnCallNode::FnCallNode(std::string identifier, FnDeclNode *fnDecl)
        : identifier(identifier),
          fnDecl(fnDecl) {
    verifyParameters();
}

void FnCallNode::verifyParameters() {
    if (identifier == "svc" && argList.size() >= 1) {
        return;
    }

    if (argList.size() != fnDecl->paramList.size()) {
        goto error_mismatched_params;
    }

    // HACK: don't verify types because we want type coercion
    // for (int i = 0; i < argList.size(); i++) {
    //     if (*argList[i]->type != *fnDecl->paramList[i]->type) {
    //         goto error_mismatched_params;
    //     }
    // }

    return;
error_mismatched_params:
    std::cerr << "ERROR: Function call " << identifier
              << " does not match the declared signature\n";
    exit(EXIT_FAILURE);
}

std::ostream &operator<<(std::ostream &os, FnCallNode &node) {
    IndentedStream ios(os);
    os << "FnCallNode: " << node.identifier << "(";
    if (!node.argList.empty()) { os << '\n'; }
    for (auto &arg : node.argList) {
        ios << *arg << '\n';
    }
    return os << ')';
}
