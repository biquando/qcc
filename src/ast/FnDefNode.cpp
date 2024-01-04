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
    ios << ".align 4\n";
    cs.os << "_" << identifier << ":\n";


    cs.pushFrame(16);
    StackFrame *sf = cs.getTopFrame();
    bool containsFnCalls = false;

    std::string statementsOutput = "";
    for (auto *sNode : block) {
        if (sNode->kind == StatementNode::FnCall) {
            containsFnCalls = true;
            continue; // TODO:
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
    containsFnCalls |= sf->containsFnCalls;

    if (containsFnCalls || sf->maxStackPos > 16) { // TODO: make this better
        ios << "sub sp, sp, #" << sf->maxStackPos << '\n';
        ios << "stp fp, lr, [fp, #-16]\n";
    }
    ios << statementsOutput;
    if (containsFnCalls || sf->maxStackPos > 16) {
        ios << "ldp fp, lr, [fp, #-16]\n";
        ios << "add sp, sp, #" << sf->maxStackPos << '\n';
    }

    cs.popFrame();


    ios << "ret\n";
    cs.os << '\n';
}
