#include <iostream>
#include <string>
#include "ast/ast.hpp"
#include "CompileState.hpp"
#include "util.hpp"

IfNode::IfNode(ExprNode *condition)
        : StatementNode(If),
          condition(condition) {}

IfNode::IfNode(ExprNode *condition, std::vector<StatementNode *> block)
        : StatementNode(If),
          condition(condition),
          block(block) {}

IfNode::IfNode(ExprNode *condition,
               std::vector<StatementNode *> block,
               std::vector<StatementNode *> elseBlock)
        : StatementNode(If),
          condition(condition),
          block(block),
          elseBlock(elseBlock) {}

/*
	ldr x16, [fp, #-24] ; where fp,#-24 is the condition expression
	cmp x16, #0
	cset x16, eq
	tbnz x16, #0, IF_FALSE_0
	b IF_TRUE_0
	IF_TRUE_0:
	b IF_EXIT_0
	IF_FALSE_0:
	b IF_EXIT_0
	IF_EXIT_0:
*/
std::string IfNode::emit(StackFrame *sf) {
    std::string output = "";
    const unsigned long labelId = (sf->cs->numIfs)++;
    const std::string labelIdStr = std::to_string(labelId);

    TypeNode condType = TypeNode(LiteralType::Int);
    auto condRes = StackFrame::Reservation(&condType, Register::x16);

    output += condRes.emitFromExprNode(sf, condition);
    output += "cmp x16, #0\n"
              "cset x16, eq\n"
              "tbnz x16, #0, IF_FALSE_" + labelIdStr + "\n"
              "b IF_TRUE_" + labelIdStr + "\n"
              "IF_TRUE_" + labelIdStr + ":\n";

    for (auto *statement : block) {
        output += statement->emit(sf);
    }

    output += "b IF_EXIT_" + labelIdStr + "\n"
              "IF_FALSE_" + labelIdStr + ":\n";
    for (auto *statement : elseBlock) {
        output += statement->emit(sf);
    }

    output += "b IF_EXIT_" + labelIdStr + "\n"
              "IF_EXIT_" + labelIdStr + ":\n";
    return output;
}

bool IfNode::containsFnCalls() {
    if (condition->containsFnCalls()) { return true; }
    for (auto *statement : block) {
        if (statement->containsFnCalls()) { return true; }
    }
    return false;
}

std::ostream &operator<<(std::ostream &os, IfNode &node) {
    IndentedStream ios(os);
    os << "IfNode (\n";
    ios << *(node.condition);

    os << "\n) {";
    if (!node.block.empty()) {
        ios << '\n';
    }
    for (auto *statement : node.block) {
        ios << *statement << '\n';
    }

    os << "} else {";
    if (!node.elseBlock.empty()) {
        ios << '\n';
    }
    for (auto *statement : node.elseBlock) {
        ios << *statement << '\n';
    }
    return os << '}';
}
