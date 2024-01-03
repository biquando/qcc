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

std::string StackFrame::Reservation::emitCopyTo(Reservation other) {
    std::string output = "";
    Register regFrom, regTo;
    Reservation *from = nullptr, *to = nullptr;

    if (kind == Reg) {
        regFrom = location.reg;
        from = this;
    } else {
        regFrom = Register::x16;
        from = new Reservation(type, regFrom);
    }

    if (other.kind == Reg) {
        regTo = other.location.reg;
        to = &other;
    } else {
        regTo = Register::x17;
        to = new Reservation(other.type, regTo);
    }

    if (kind == Stack) { delete from; }
    if (other.kind == Stack) { delete to; }

    if (kind == Reg && other.kind == Reg) {
        output += "mov " + toStr(other.location.reg) + ", "
                + toStr(location.reg) + "\n";
    } else if (kind == Reg && other.kind == Stack) {
        output += "str " + toStr(location.reg) + ", "
                + "[fp, #-" + std::to_string(other.location.stackOffset)
                + "]\n";
                // FIXME: use dynamic offsets
    } // TODO: other cases

    return output;
}

std::string StackFrame::Reservation::emitPutValue(unsigned long val) {
    std::string output = "";
    Register reg;
    Reservation *res = nullptr;
    if (kind == Reg) {
        reg = location.reg;
    } else {
        reg = Register::x16; // FIXME: this register might be in use
        res = new Reservation(nullptr, reg); // FIXME: change nullptr
    }

    output += "mov " + toStr(reg) + ", #"
            + std::to_string(val & 0xffff) + "\n";
    if (val & 0x00000000ffff0000l) {
        output += "movk " + toStr(reg) + ", #"
                + std::to_string((val >> 16) & 0xffff)
                + ", LSL #16\n";
    }
    if (val & 0x0000ffff00000000l) {
        output += "movk " + toStr(reg) + ", #"
                + std::to_string((val >> 32) & 0xffff)
                + ", LSL #32\n";
    }
    if (val & 0xffff000000000000l) {
        output += "movk " + toStr(reg) + ", #"
                + std::to_string((val >> 48) & 0xffff)
                + ", LSL #48\n";
    }

    if (kind == Stack) {
        output += res->emitCopyTo(*this);
        delete res;
    }
    return output;
}

std::string StackFrame::Reservation::emitFromExprNode(StackFrame *sf,
                                                      ExprNode *expr) {
    std::string output = "";
    unsigned long val;
    Reservation var;
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
            if (!var.valid) { // TEMP
                std::cerr << "ERROR: Undefined variable: "
                          << expr->identifier << '\n';
                exit(1);
            }
            output += var.emitCopyTo(*this);
            break;
        case ExprNode::FnCall:
            // TODO:
            sf->containsFnCalls = true;
            break;
        case ExprNode::BinaryOp:
            // TODO:
            break;
        case ExprNode::UnaryOp:
            output += emitFromExprNode(sf, expr->opr);
            output += sf->emitUnaryOp(expr->builtinOperator, *this, *this);
    }
    return output;
}

/* SECTION: StackFrame */

StackFrame::StackFrame(long initialStackPos)
        : stackPos(initialStackPos),
          maxStackPos(initialStackPos) {}

void StackFrame::addVariable(TypeNode *type, std::string identifier) {
    Reservation res = pushReservation(type); // TODO: prefer to push to stack
    variables[identifier] = res;
}

StackFrame::Reservation StackFrame::getVariable(std::string identifier) {
    return variables[identifier];
}

StackFrame::Reservation StackFrame::pushReservation(TypeNode *type) {
    int nReserved = reservations.size();
    if (nReserved < 8) {
        reservations.emplace_back(type, (Register)(nReserved + 8));
    } else {
        stackPos += 8;
        if (stackPos > maxStackPos) { maxStackPos = stackPos; }
        reservations.emplace_back(type, stackPos);
    }
    return reservations.back();
}

void StackFrame::popReservation() {
    reservations.pop_back();
}

// TODO: Implement emitBinaryOp

std::string StackFrame::emitUnaryOp(BuiltinOperator op, Reservation res,
                                    Reservation opr) {
    std::string output = "";
    Register resRegister, oprRegister;
    Reservation resReservation, oprReservation;

    if (res.kind == Reservation::Reg) {
        resRegister = res.location.reg;
    } else {
        resRegister = Register::x16; // FIXME: this register might be in use
        resReservation = Reservation(res.type, resRegister);
    }

    if (opr.kind == Reservation::Reg) {
        oprRegister = opr.location.reg;
    } else {
        oprRegister = Register::x17; // FIXME: this register might be in use
        oprReservation = Reservation(res.type, oprRegister);
        output += opr.emitCopyTo(oprReservation);
    }

    switch (op) {
        case BuiltinOperator::Minus:
            output += "neg " + toStr(resRegister) + ", "
                    + toStr(oprRegister) + "\n";
            break;
        case BuiltinOperator::Not:
            output += "mvn " + toStr(resRegister) + ", "
                    + toStr(oprRegister) + "\n";
            break;
        default:
            break;
    }

    if (res.kind == Reservation::Stack) {
        output += resReservation.emitCopyTo(res);
    }

    return output;
}

