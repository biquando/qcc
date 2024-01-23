#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include "CompileState.hpp"

StackFrame::StackFrame(CompileState *cs, FnDefNode *fnDef)
        : cs(cs),
          fnDef(fnDef) {}

void StackFrame::incStackPos(long amt) {
    stackPos += amt;

    if (amt > 0) {
        long paddingAmt = 0;
        while (stackPos % amt != 0) {
            paddingAmt++;
            stackPos++;
        }
        stackIncrementPadding.push_back(paddingAmt);
    } else if (amt < 0) {
        stackPos -= stackIncrementPadding.back();
        stackIncrementPadding.pop_back();
    }

    if (stackPos > maxStackPos) {
        maxStackPos = stackPos;
    }
}

void StackFrame::addVariable(TypeNode *type, std::string identifier) {
    Reservation res = reserveVariable(type);
    variables[identifier] = res;
}

StackFrame::Reservation StackFrame::getVariable(std::string identifier) {
    if (variables.find(identifier) == variables.end()) {
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
    incStackPos(type->size());
    variableReservations.emplace_back(type, stackPos);
    return variableReservations.back();
}

StackFrame::Reservation StackFrame::reserveExpr(TypeNode *type) {
    int numReservedExpr = exprReservations.size();
    if (numReservedExpr < 8) {
        exprReservations.emplace_back(type, (Register)(numReservedExpr + 8));
    } else {
        incStackPos(type->size());
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
    incStackPos(-(long)variableReservations.back().type->size());
    variableReservations.pop_back();
}

void StackFrame::unreserveExpr() {
    if (exprReservations.size() == 0) { return; }

    Reservation latest = exprReservations.back();
    if (latest.kind == Reservation::Stack) {
        incStackPos(latest.type->size());
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
        case BuiltinOperator::Star: {
            std::string ldrInstr, r;
            switch (res.type->size()) {
                case 1:
                    ldrInstr = "ldrb";
                    r = "w";
                    break;
                default:
                    ldrInstr = "ldr";
                    r = "x";
            }
            output += ldrInstr + " " + toStr(dst.location.reg, r) + ", ["
                    + toStr(src.location.reg) + "]\n";
            break;
        }
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

std::string StackFrame::emitAddressOf(Reservation res, std::string identifier) {
    Reservation var = getVariable(identifier);
    long stackOffset = var.location.stackOffset;
    std::string output = "";
    Reservation dst;

    if (res.kind == Reservation::Reg) {
        dst = res;
    } else {
        dst = Reservation(res.type, Register::x16);
    }

    output += "mov " + toStr(dst.location.reg) + ", #"
            + toStr(stackOffset) + "\n";
    output += "sub " + toStr(dst.location.reg) + ", fp, "
            + toStr(dst.location.reg) + "\n";

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
        output += "str " + toStr((Register)(numToSave+7)) + ", [sp, #-16]!\n";
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
