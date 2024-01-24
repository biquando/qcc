#include <string>
#include "ast.hpp"

BreakNode::BreakNode() : StatementNode(Break) {}

std::string BreakNode::emit(StackFrame *sf) {
    std::string labelIdStr = std::to_string(sf->loopIds.back());
    return "b WHILE_EXIT_" + labelIdStr + "\n";
}
