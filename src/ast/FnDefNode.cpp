#include "ast/ast.hpp"
#include "util.hpp"
#include "CompileState.hpp"

FnDefNode::FnDefNode(FnDeclNode fnDeclNode, std::vector<StatementNode *> block)
        : FnDeclNode(fnDeclNode.returnType,
                     fnDeclNode.identifier,
                     fnDeclNode.paramList),
          block(block) {}

std::ostream &operator<<(std::ostream &os, FnDefNode &node) {
    IndentedStream ios(os);
    os << "FnDefNode: (";
    os << *(node.returnType) << ") " << node.identifier << '(';

    if (!node.paramList.empty()) { os << '\n'; }
    for (auto &paramNode : node.paramList) {
        ios << *paramNode << '\n';
    }
    os << ") {";

    if (!node.block.empty()) { os << '\n'; }
    for (auto &statementNode : node.block) {
        ios << *statementNode << '\n';
    }
    return os << '}';
}

void FnDefNode::emit(CompileState &cs) {
    IndentedStream ios(cs.os, cs.indent);
    ios << ".globl _" << identifier << '\n';
    ios << ".p2align 2\n";
    cs.os << "_" << identifier << ":\n";

    bool containsFnCalls = false;
    for (StatementNode *sNode : block) {
        containsFnCalls |= sNode->containsFnCalls();
    }

    StackFrame *sf = cs.frames.at(identifier);
    const long fnCallOffset = containsFnCalls ? 16 : 0;

    std::string statementsOutput = "";
    for (int i = 0; i < paramList.size() && i < 8; i++) { // TODO: support more than 8 arguments
        ParamNode *param = paramList[i];
        sf->addVariable(param->type, param->identifier);

        StackFrame::Reservation to = sf->getVariable(param->identifier);
        auto from = StackFrame::Reservation(param->type, (Register)i);
        statementsOutput += from.emitCopyTo(to);
    }

    if (identifier == "main") {
        auto *zero = new LiteralNode(0l);
        auto *retVal = new ExprNode(zero);
        auto *retStatement = new StatementNode(retVal);
        block.push_back(retStatement);
    }

    for (auto *sNode : block) {
        statementsOutput += sNode->emit(sf);
    }
    while (sf->maxStackPos % 16 != 0) {
        sf->maxStackPos += 1;
    }

    if (sf->maxStackPos > 0 || containsFnCalls) {
        ios << "sub sp, sp, #" << sf->maxStackPos + fnCallOffset << '\n';
    }
    if (containsFnCalls) {
        ios << "stp fp, lr, [sp, #" << sf->maxStackPos << "]\n"
            << "sub fp, fp, #16\n";
    }

    ios << statementsOutput;
    cs.os << "return_" << identifier << ":\n";

    if (containsFnCalls) {
        ios << "ldp fp, lr, [sp, #" << sf->maxStackPos << "]\n";
    }
    if (sf->maxStackPos > 0 || containsFnCalls) {
        ios << "add sp, sp, #" << sf->maxStackPos + fnCallOffset << '\n';
    }

    ios << "ret\n";
    cs.os << '\n';
}
