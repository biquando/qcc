#include <iostream>
#include <string>
#include "ast/ast.hpp"
#include "CompileState.hpp"
#include "util.hpp"


WhileNode::WhileNode(ExprNode *condition, std::vector<StatementNode *> block)
        : StatementNode(While),
          condition(condition),
          block(block) {}

/*
WHILE_COND_0:
    ldr x16, [fp #-24] ; where fp,#-24 is the condition expression
    cmp x16, #0
    cset x16, eq
    tbnz x16, #0, WHILE_EXIT_0
    b WHILE_BODY_0
WHILE_BODY_0:
    ; (body of loop)
    b WHILE_COND_0
WHILE_EXIT_0:
    ; (after the loop)
*/
std::string WhileNode::emit(StackFrame *sf) {
    std::string output = "";
    const unsigned long labelId = (sf->cs->numWhiles)++;
    sf->loopIds.push_back(labelId);
    const std::string labelIdStr = std::to_string(labelId);

    TypeNode condType = TypeNode(LiteralType::Int);
    auto condRes = StackFrame::Reservation(&condType, Register::x16);

    output += "WHILE_COND_" + labelIdStr + ":\n";
    output += condRes.emitFromExprNode(sf, condition);
    output += "cmp x16, #0\n"
              "cset x16, eq\n"
              "tbnz x16, #0, WHILE_EXIT_" + labelIdStr + "\n"
              "b WHILE_BODY_" + labelIdStr + "\n"
              "WHILE_BODY_" + labelIdStr + ":\n";

    for (auto *statement : block) {
        output += statement->emit(sf);
    }
    output += "b WHILE_COND_" + labelIdStr + "\n"
              "WHILE_EXIT_" + labelIdStr + ":\n";

    sf->loopIds.pop_back();
    return output;
}

bool WhileNode::containsFnCalls() {
    if (condition->containsFnCalls()) { return true; }
    for (auto *statement : block) {
        if (statement->containsFnCalls()) { return true; }
    }
    return false;
}

std::ostream &operator<<(std::ostream &os, WhileNode &node) {
    IndentedStream ios(os);
    os << "WhileNode (\n";
    ios << *(node.condition);

    os << "\n) {";
    if (!node.block.empty()) {
        ios << '\n';
    }
    for (auto *statement : node.block) {
        ios << *statement << '\n';
    }
    return os << '}';
}
