#include "ast/ast.hpp"
#include "util.hpp"
#include "CompileState.hpp"

FnDefNode::FnDefNode(TypeNode *returnType,
              std::string identifier,
              std::vector<StatementNode *> block)
        : returnType(returnType),
          identifier(identifier),
          block(block) {}

FnDefNode::FnDefNode(TypeNode *returnType,
              std::string identifier,
              std::vector<ParamNode *> paramList,
              std::vector<StatementNode *> block)
        : returnType(returnType),
          identifier(identifier),
          paramList(paramList),
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

    cs.pushFrame();
    StackFrame *sf = cs.getTopFrame();
    if (containsFnCalls) {
        sf->incStackPos(16);
    }

    std::string statementsOutput = "";
    for (int i = 0; i < paramList.size() && i < 8; i++) { // TODO: support more than 8 arguments
        ParamNode *param = paramList[i];
        sf->addVariable(param->type, param->identifier);

        StackFrame::Reservation to = sf->getVariable(param->identifier);
        auto from = StackFrame::Reservation(param->type, (Register)i);
        statementsOutput += from.emitCopyTo(to);
    }

    for (auto *sNode : block) {
        if (sNode->kind == StatementNode::FnCall) {
            // TODO: types, and allow more than 8 arguments
            FnCallNode *fnCall = sNode->fnCall;
            for (int i = 0; i < fnCall->argList.size() && i < 8; i++) {
                auto res = StackFrame::Reservation(nullptr, (Register)i);
                statementsOutput += res.emitFromExprNode(sf, fnCall->argList[i]);
            }
            statementsOutput += sf->emitSaveCaller();
            statementsOutput += "bl _" + fnCall->identifier + "\n";
            statementsOutput += sf->emitLoadCaller();
            continue;
        }

        if (sNode->kind == StatementNode::Return) {
            auto ret = StackFrame::Reservation(nullptr, Register::x0); // TODO: types
            if (sNode->containsFnCalls()) {
                auto tmpRes = sf->reserveExpr(nullptr); // TODO: types
                statementsOutput += tmpRes.emitFromExprNode(sf, sNode->expr);
                statementsOutput += tmpRes.emitCopyTo(ret);
            } else {
                statementsOutput += ret.emitFromExprNode(sf, sNode->expr);
            }
            statementsOutput += "b return_" + identifier + "\n";
            continue;
        }

        if (sNode->kind == StatementNode::Declaration
         || sNode->kind == StatementNode::Initialization) {
            sf->addVariable(sNode->type, sNode->identifier);
        }

        // assuming not fncall
        if (sNode->kind == StatementNode::Initialization
         || sNode->kind == StatementNode::Assignment) {
            StackFrame::Reservation var = sf->getVariable(sNode->identifier);
            statementsOutput += var.emitFromExprNode(sf, sNode->expr);
        }
    }
    while (sf->maxStackPos % 16 != 0) {
        sf->maxStackPos += 1;
    }

    if (sf->maxStackPos > 0) {
        ios << "sub sp, sp, #" << sf->maxStackPos << '\n';
    }
    if (containsFnCalls) {
        ios << "stp fp, lr, [sp, #" << sf->maxStackPos - 16 << "]\n";
    }

    ios << statementsOutput;
    cs.os << "return_" << identifier << ":\n";

    if (containsFnCalls) {
        ios << "ldp fp, lr, [sp, #" << sf->maxStackPos - 16 << "]\n";
    }
    if (sf->maxStackPos > 0) {
        ios << "add sp, sp, #" << sf->maxStackPos << '\n';
    }

    cs.popFrame();
    ios << "ret\n";
    cs.os << '\n';
}
