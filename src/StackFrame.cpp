#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "CompileState.hpp"

StackFrame::Reservation::Reservation(TypeNode *type, Register reg)
        : kind(Reg),
          type(type) {
    location.reg = reg;
}

StackFrame::Reservation::Reservation(TypeNode *type, long stackOffset)
        : kind(Stack),
          type(type) {
    location.stackOffset = stackOffset;
}

StackFrame::Reservation::Reservation()
        : valid(false) {}

bool StackFrame::Reservation::operator==(Reservation &other) {
    if (kind != other.kind) { return false; }
    if (kind == Reg) {
        return location.reg == other.location.reg;
    }
    if (kind == Stack) {
        return location.stackOffset == other.location.stackOffset;
    }
    return false;
}

std::string StackFrame::Reservation::emitCopyTo(Reservation other) {
    if (*this == other) {
        return "";
    }

    std::string output = "";

    if (kind == Reg && other.kind == Reg) {
        output += "mov " + toStr(other.location.reg) + ", "
                + toStr(location.reg) + "\n";

    } else if (kind == Reg && other.kind == Stack) {
        output += "str " + toStr(location.reg) + ", "
                + "[fp, #-" + toStr(other.location.stackOffset)
                + "]\n";

    } else if (kind == Stack && other.kind == Reg) {
        output += "ldr " + toStr(other.location.reg) + ", "
                + "[fp, #-" + toStr(location.stackOffset)
                + "]\n";

    } else if (kind == Stack && other.kind == Stack) {
        const Register TMP_REG = Register::x16;
        output += "ldr " + toStr(TMP_REG) + ", "
                + "[fp, #-" + toStr(location.stackOffset)
                + "]\n";
        output += "str " + toStr(TMP_REG) + ", "
                + "[fp, #-" + toStr(other.location.stackOffset)
                + "]\n";
    }

    return output;
}

std::string StackFrame::Reservation::emitPutValue(unsigned long val) {
    std::string output = "";
    Reservation res;
    if (kind == Reg) {
        res = *this;
    } else {
        res = Reservation(type, Register::x16);
    }

    output += "mov " + toStr(res.location.reg) + ", #"
            + std::to_string(val & 0xffff) + "\n";
    if (val & 0x00000000ffff0000l) {
        output += "movk " + toStr(res.location.reg) + ", #"
                + std::to_string((val >> 16) & 0xffff)
                + ", LSL #16\n";
    }
    if (val & 0x0000ffff00000000l) {
        output += "movk " + toStr(res.location.reg) + ", #"
                + std::to_string((val >> 32) & 0xffff)
                + ", LSL #32\n";
    }
    if (val & 0xffff000000000000l) {
        output += "movk " + toStr(res.location.reg) + ", #"
                + std::to_string((val >> 48) & 0xffff)
                + ", LSL #48\n";
    }

    if (kind == Stack) {
        output += res.emitCopyTo(*this);
    }
    return output;
}

std::string StackFrame::Reservation::emitFromExprNode(StackFrame *sf,
                                                      ExprNode *expr) {
    std::string output = "";
    unsigned long val;
    Reservation var, exprRes;
    switch(expr->kind) {
        case ExprNode::Literal:
            switch (expr->literal->type) {
                case LiteralType::Int:
                    val = expr->literal->i;
                    break;
                case LiteralType::Char:
                    val = expr->literal->c;
                    break;
            }
            output += emitPutValue(val);
            break;
        case ExprNode::Identifier:
            var = sf->getVariable(expr->identifier);
            output += var.emitCopyTo(*this);
            break;
        case ExprNode::FnCall: {
            // TODO: types, and allow more than 8 arguments
            FnCallNode *fnCall = expr->fnCall;
            for (int i = 0; i < fnCall->argList.size() && i < 8; i++) {
                auto res = StackFrame::Reservation(nullptr, (Register)i);
                output += res.emitFromExprNode(sf, fnCall->argList[i]);
            }
            auto returnVal = StackFrame::Reservation(nullptr, Register::x0);
            output += sf->emitSaveCaller();
            output += "bl _" + fnCall->identifier + "\n";
            output += sf->emitLoadCaller();
            output += returnVal.emitCopyTo(*this);
            break;
        }
        case ExprNode::BinaryOp:
            exprRes = sf->reserveExpr(nullptr); // TODO: use actual types
            output += emitFromExprNode(sf, expr->opr1);
            output += exprRes.emitFromExprNode(sf, expr->opr2);
            output += sf->emitBinaryOp(expr->builtinOperator, *this, *this, exprRes);
            sf->unreserveExpr();
            break;
        case ExprNode::UnaryOp:
            output += emitFromExprNode(sf, expr->opr);
            output += sf->emitUnaryOp(expr->builtinOperator, *this, *this);
            break;
    }
    return output;
}

/* SECTION: StackFrame */

StackFrame::StackFrame() {}

void StackFrame::incStackPos(long amt) {
    stackPos += amt;
    if (stackPos > maxStackPos) {
        maxStackPos = stackPos;
    }
}

void StackFrame::addVariable(TypeNode *type, std::string identifier) {
    Reservation res = reserveVariable(type);
    variables[identifier] = res;
}

StackFrame::Reservation StackFrame::getVariable(std::string identifier) {
    if (variables.find(identifier) == variables.end()) { // TEMP
        std::cerr << "ERROR: Undefined variable: "
                  << identifier << '\n';
        exit(EXIT_FAILURE);
    }
    return variables[identifier];
}

StackFrame::Reservation StackFrame::reserveVariable(TypeNode *type) {
    if (exprReservations.size() > 0) {
        std::cerr << "COMPILER ERROR: Can't reserve variable while expressions "
                     "are still reserved\n";
        exit(EXIT_FAILURE);
    }
    incStackPos(8);
    variableReservations.emplace_back(type, stackPos);
    return variableReservations.back();
}

StackFrame::Reservation StackFrame::reserveExpr(TypeNode *type) {
    int numReservedExpr = exprReservations.size();
    if (numReservedExpr < 8) {
        exprReservations.emplace_back(type, (Register)(numReservedExpr + 8));
    } else {
        incStackPos(8);
        exprReservations.emplace_back(type, stackPos);
    }
    return exprReservations.back();
}

void StackFrame::unreserveVariable() {
    if (exprReservations.size() > 0) {
        std::cerr << "COMPILER ERROR: Can't unreserve variable while "
                     "expressions are still reserved\n";
        exit(EXIT_FAILURE);
    }
    incStackPos(-8);
    variableReservations.pop_back();
}

void StackFrame::unreserveExpr() {
    if (exprReservations.size() == 0) { return; }

    Reservation latest = exprReservations.back();
    if (latest.kind == Reservation::Stack) {
        incStackPos(-8);
    }
    exprReservations.pop_back();
}

std::string StackFrame::emitBinaryOp(BuiltinOperator op, Reservation res,
                                     Reservation opr1, Reservation opr2) {
    std::string output = "";
    Reservation dst, src;

    if (res.kind == Reservation::Reg) {
        dst = res;
    } else {
        dst = Reservation(res.type, Register::x16);
    }
    output += opr1.emitCopyTo(dst);

    if (opr2.kind == Reservation::Reg) {
        src = opr2;
    } else {
        src = Reservation(opr2.type, Register::x17);
        output += opr2.emitCopyTo(src);
    }

    switch(op) {
        case BuiltinOperator::Plus:
            output += "add " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::Minus:
            output += "sub " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::Star:
            output += "mul " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::Fslash:
            output += "sdiv " + toStr(dst.location.reg) + ", " // TODO: assumes signed
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::Eq:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", eq\n";
            break;
        case BuiltinOperator::Ne:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", ne\n";
            break;
        case BuiltinOperator::Lt:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", lt\n"; // TODO: assumes signed
            break;
        case BuiltinOperator::Gt:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", gt\n"; // TODO: assumes signed
            break;
        case BuiltinOperator::Le:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", le\n"; // TODO: assumes signed
            break;
        case BuiltinOperator::Ge:
            output += "cmp " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            output += "cset " + toStr(dst.location.reg) + ", ge\n"; // TODO: assumes signed
            break;
        case BuiltinOperator::BitAnd:
            output += "and " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::BitOr:
            output += "orr " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::BitXor:
            output += "eor " + toStr(dst.location.reg) + ", "
                    + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        default:
            break;
    }

    output += dst.emitCopyTo(res);
    return output;
}

std::string StackFrame::emitUnaryOp(BuiltinOperator op, Reservation res,
                                    Reservation opr) {
    std::string output = "";
    Reservation dst, src;

    if (res.kind == Reservation::Reg) {
        dst = res;
    } else {
        dst = Reservation(res.type, Register::x16);
    }

    if (opr.kind == Reservation::Reg) {
        src = opr;
    } else {
        src = Reservation(opr.type, Register::x17);
        output += opr.emitCopyTo(src);
    }

    switch (op) {
        case BuiltinOperator::Minus:
            output += "neg " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        case BuiltinOperator::Not:
            output += "cmp " + toStr(src.location.reg) + ", #0\n";
            output += "cset " + toStr(dst.location.reg) + ", eq\n";
            break;
        case BuiltinOperator::BitNot:
            output += "mvn " + toStr(dst.location.reg) + ", "
                    + toStr(src.location.reg) + "\n";
            break;
        default:
            break;
    }

    output += dst.emitCopyTo(res);
    return output;
}

std::string StackFrame::emitSaveCaller() {
    std::string output = "";
    int numToSave = exprReservations.size() < 8 ? exprReservations.size() : 8;
    for (int i = 1; i < numToSave; i += 2) {
        output += "stp " + toStr((Register)(i+7)) + ", "
                + toStr((Register)(i+8)) + ", [sp, #-16]!\n";
    }
    if (numToSave % 2 == 1) {
        output += "str " + toStr((Register)(numToSave+7)) + ", [sp, #-16]\n";
    }
    return output;
}

std::string StackFrame::emitLoadCaller() {
    std::string output = "";
    int numToLoad = exprReservations.size() < 8 ? exprReservations.size() : 8;
    if (numToLoad % 2 == 1) {
        output += "ldr " + toStr((Register)(numToLoad+7)) + ", [sp], #16\n";
    }
    for (int i = numToLoad - 1 - (numToLoad % 2); i > 0; i -= 2) {
        output += "ldp " + toStr((Register)(i+7)) + ", "
                + toStr((Register)(i+8)) + ", [sp], #16\n";
    }
    return output;
}
