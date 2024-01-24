#include <string>
#include "ast.hpp"

ContinueNode::ContinueNode() : StatementNode(Continue) {}

std::string ContinueNode::emit(StackFrame *sf) {
    std::string labelIdStr = std::to_string(sf->loopIds.back());
    return "b WHILE_COND_" + labelIdStr + "\n";
}
