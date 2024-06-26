#include "ast/ast.hpp"
#include "util.hpp"

StatementNode::StatementNode(TypeNode *type, std::string identifier)
        : kind(Declaration),
          type(type),
          identifier(identifier) {
    if (*type == TypeNode(BuiltinType::Void)) {
        std::cerr << "ERROR: Can't declare variable with void type\n";
        exit(EXIT_FAILURE);
    }
}

StatementNode::StatementNode(TypeNode *type, std::string identifier, ExprNode *expr)
        : kind(Initialization),
          type(type),
          identifier(identifier),
          expr(expr) {
    if (*type == TypeNode(BuiltinType::Void)) {
        std::cerr << "ERROR: Can't declare variable with void type\n";
        exit(EXIT_FAILURE);
    }

    if (expr->kind == ExprNode::Array && type->kind != TypeNode::Pointer) {
        std::cerr << "ERROR: Can't assign array to variable of type ("
                  << *type << ")\n";
        exit(EXIT_FAILURE);
    }
}

StatementNode::StatementNode(AccessorNode *accessor, ExprNode *rexpr)
        : kind(Assignment),
          accessor(accessor),
          expr(rexpr) {}

StatementNode::StatementNode(ExprNode *returnExpr)
        : kind(Return),
          expr(returnExpr) {}

StatementNode::StatementNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall) {}

StatementNode::StatementNode(StatementKind derivedKind)
        : kind(derivedKind) {
    if (!isDerived()) {
        std::cerr << "COMPILER ERROR: Tried to initialize StatementNode to "
                     "a non-derived kind, but didn't use the appropriate "
                     "constructor\n";
        exit(EXIT_FAILURE);
    }
}

std::string StatementNode::emit(StackFrame *sf) {
    std::string output = "";

    if (kind == StatementNode::FnCall && fnCall->identifier == "svc") {
        for (int i = 1; i < fnCall->argList.size() && i < 8; i++) {
            ExprNode *argNode = fnCall->argList[i];
            auto arg = StackFrame::Reservation(argNode->type, (Register)(i-1));
            if (argNode->containsFnCalls()) {
                auto tmpRes = sf->reserveExpr(argNode->type);
                output += tmpRes.emitFromExprNode(sf, argNode);
                output += tmpRes.emitCopyTo(arg);
                sf->unreserveExpr();
            } else {
                output += arg.emitFromExprNode(sf, argNode);
            }
        }
        TypeNode intType = TypeNode(BuiltinType::Int);
        auto syscallRes = StackFrame::Reservation(&intType, Register::x16);
        output += syscallRes.emitFromExprNode(sf, fnCall->argList[0]);
        output += "svc #0\n";
        goto endStatement;
    }

    if (kind == StatementNode::FnCall) {
        // Check for builtin functions
        for (auto &builtin : BUILTIN_FNS) {
            const std::string &name = builtin.second;
            if (name == fnCall->identifier) {
                sf->cs->usedBuiltinFns.insert(builtin.first);
            }
        }

        // TODO: use fnDef types instead of fnCall types, and allow more than 8 arguments
        for (int i = 0; i < fnCall->argList.size() && i < 8; i++) {
            ExprNode *argNode = fnCall->argList[i];
            auto arg = StackFrame::Reservation(argNode->type, (Register)i);
            if (argNode->containsFnCalls()) {
                auto tmpRes = sf->reserveExpr(argNode->type);
                output += tmpRes.emitFromExprNode(sf, argNode);
                output += tmpRes.emitCopyTo(arg);
                sf->unreserveExpr();
            } else {
                output += arg.emitFromExprNode(sf, argNode);
            }
        }
        output += sf->emitSaveCaller();
        output += "bl _" + fnCall->identifier + "\n";
        output += sf->emitLoadCaller();
        goto endStatement;
    }

    if (kind == StatementNode::Return) {
        if (expr->kind != ExprNode::Empty) {
            auto ret = StackFrame::Reservation(expr->type, Register::x0);
            if (containsFnCalls()) {
                auto tmpRes = sf->reserveExpr(expr->type);
                output += tmpRes.emitFromExprNode(sf, expr);
                output += tmpRes.emitCopyTo(ret);
            } else {
                output += ret.emitFromExprNode(sf, expr);
            }
        }
        output += "b return_" + sf->fnDef->identifier + "\n";
        goto endStatement;
    }

    if (kind == StatementNode::Declaration) {
        sf->addVariable(type, identifier);
        goto endStatement;
    }

    if (kind == StatementNode::Initialization) {
        sf->addVariable(type, identifier);
        StackFrame::Reservation var = sf->getVariable(identifier);
        output += var.emitFromExprNode(sf, expr);
        goto endStatement;
    }

    if (kind == StatementNode::Assignment && accessor->kind == AccessorNode::Identifier) {
        StackFrame::Reservation varRes = sf->getVariable(accessor->identifier);
        StackFrame::Reservation valRes = sf->reserveExpr(varRes.type);
        output += valRes.emitFromExprNode(sf, expr);
        output += valRes.emitCopyTo(varRes);
        sf->unreserveExpr();
        goto endStatement;
    }

    if (kind == StatementNode::Assignment && accessor->kind == AccessorNode::Dereference) {
        if (accessor->expr->type->kind != TypeNode::Pointer) {
            std::cerr << "ERROR: Tried to dereference non-pointer type "
                      << *(accessor->expr->type) << '\n';
            exit(EXIT_FAILURE);
        }

        StackFrame::Reservation ptrRes = sf->reserveVariable(accessor->expr->type);
        output += ptrRes.emitFromExprNode(sf, accessor->expr);
        StackFrame::Reservation valRes = sf->reserveVariable(expr->type);
        output += valRes.emitFromExprNode(sf, expr);

        StackFrame::Reservation tmpPtrRes(ptrRes.type, Register::x16);
        output += ptrRes.emitCopyTo(tmpPtrRes);
        StackFrame::Reservation tmpValRes(ptrRes.type, Register::x17);
        output += valRes.emitCopyTo(tmpValRes);

        std::string strInstr, r;
        switch (ptrRes.type->pointerType->size()) {
             case 1:
                 strInstr = "strb";
                 r = "w";
                 break;
             default:
                 strInstr = "str";
                 r = "x";
        }
        output += strInstr + " " + toStr(tmpValRes.location.reg, r) + ", ["
                  + toStr(tmpPtrRes.location.reg) + "]\n";

        sf->unreserveVariable();  // unreserve valRes
        sf->unreserveVariable();  // unreserve ptrRes
        goto endStatement;
    }

    std::cerr << "COMPILER ERROR: Tried to emit StatementNode with "
                 "invalid kind\n";
    exit(EXIT_FAILURE);

endStatement:
    return output;
}

bool StatementNode::containsFnCalls() {
    return kind == FnCall
        || (kind == Initialization || kind == Assignment || kind == Return)
            && expr->containsFnCalls();
}

bool StatementNode::isDerived() {
    return kind >= If;
}

std::ostream &operator<<(std::ostream &os, StatementNode &node) {
    IndentedStream ios(os);
    if (!node.isDerived()) {
        os << "StatementNode (";
    }
    switch (node.kind) {
        case StatementNode::Declaration:
            os << "Declaration): (";
            os << *(node.type) << ") " << node.identifier;
            break;
        case StatementNode::Initialization:
            os << "Initialization): (";
            os << *(node.type) << ") " << node.identifier;
            ios << '\n' << *(node.expr);
            break;
        case StatementNode::Assignment:
            os << "Assignment):\n";
            ios << *(node.accessor) << '\n' << *(node.expr);
            break;
        case StatementNode::Return:
            os << "Return):\n";
            ios << *(node.expr);
            break;
        case StatementNode::FnCall:
            os << "FnCall):";
            ios << '\n' << *(node.fnCall);
            break;
        case StatementNode::If: {
            IfNode *ifNode = static_cast<IfNode *>(&node);
            os << *ifNode;
            break;
        }
        case StatementNode::While: {
            WhileNode *whileNode = static_cast<WhileNode *>(&node);
            os << *whileNode;
            break;
        }
        case StatementNode::Break:
            os << "BreakNode";
            break;
        case StatementNode::Continue:
            os << "ContinueNode";
            break;
    }
    return os;
}
