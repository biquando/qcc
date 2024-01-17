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

    std::string strInstr, ldrInstr, rTo, rFrom;
    switch (type->size()) {
        case 1:
            ldrInstr = "ldrb";
            rFrom = "w";
            break;
        default:
            ldrInstr = "ldr";
            rFrom = "x";
    }
    switch (other.type->size()) {
        case 1:
            strInstr = "strb";
            rTo = "w";
            break;
        default:
            strInstr = "str";
            rTo = "x";
    }

    if (kind == Reg && other.kind == Reg) {
        output += "mov " + toStr(other.location.reg, rTo) + ", "
                + toStr(location.reg, rFrom) + "\n";

    } else if (kind == Reg && other.kind == Stack) {
        output += strInstr + " " + toStr(location.reg, rFrom) + ", "
                + "[fp, #-" + toStr(other.location.stackOffset)
                + "]\n";

    } else if (kind == Stack && other.kind == Reg) {
        output += ldrInstr + " " + toStr(other.location.reg, rTo) + ", "
                + "[fp, #-" + toStr(location.stackOffset)
                + "]\n";

    } else if (kind == Stack && other.kind == Stack) {
        const Register TMP_REG = Register::x16;
        output += ldrInstr + " " + toStr(TMP_REG, rFrom) + ", "
                + "[fp, #-" + toStr(location.stackOffset)
                + "]\n";
        output += strInstr + " " + toStr(TMP_REG, rTo) + ", "
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
            if (expr->fnCall->identifier == "svc") {
                for (int i = 1; i < expr->fnCall->argList.size() && i < 8; i++) {
                    ExprNode *argNode = expr->fnCall->argList[i];
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
                auto returnVal = StackFrame::Reservation(&intType, Register::x0);
                output += syscallRes.emitFromExprNode(sf, expr->fnCall->argList[0]);
                output += "svc #0\n";
                output += returnVal.emitCopyTo(*this);
                break;
            }
            // TODO: allow more than 8 arguments
            FnCallNode *fnCall = expr->fnCall;
            FnDeclNode *fnDecl = fnCall->fnDecl;
            for (int i = 0; i < fnCall->argList.size() && i < 8; i++) {
                ExprNode *argNode = fnCall->argList[i];
                auto arg = StackFrame::Reservation(fnDecl->paramList[i]->type, (Register)i);
                if (argNode->containsFnCalls()) {
                    auto tmpRes = sf->reserveExpr(argNode->type);
                    output += tmpRes.emitFromExprNode(sf, argNode);
                    output += tmpRes.emitCopyTo(arg);
                    sf->unreserveExpr();
                } else {
                    output += arg.emitFromExprNode(sf, argNode);
                }
            }
            auto returnVal = StackFrame::Reservation(fnDecl->returnType, Register::x0);
            output += sf->emitSaveCaller();
            output += "bl _" + fnCall->identifier + "\n";
            output += sf->emitLoadCaller();
            output += returnVal.emitCopyTo(*this);
            break;
        }
        case ExprNode::BinaryOp:
            exprRes = sf->reserveExpr(expr->type);
            output += emitFromExprNode(sf, expr->opr1);
            output += exprRes.emitFromExprNode(sf, expr->opr2);
            output += sf->emitBinaryOp(expr->builtinOperator, *this, *this, exprRes);
            sf->unreserveExpr();
            break;
        case ExprNode::UnaryOp:
            if (expr->builtinOperator == BuiltinOperator::BitAnd) {
                output += sf->emitDereference(*this, expr->opr->identifier);
            } else {
                output += emitFromExprNode(sf, expr->opr);
                output += sf->emitUnaryOp(expr->builtinOperator, *this, *this);
            }
            break;
        case ExprNode::Array:
            {
                for (int i = expr->array->size() - 1; i >= 0; i--) {
                    ExprNode *elem = (*expr->array)[i];
                    Reservation elemRes = sf->reserveVariable(elem->type);
                    output += elemRes.emitFromExprNode(sf, elem);
                }
                TypeNode ptrType(LiteralType::Int);
                Reservation tmp = Reservation(&ptrType, Register::x16);
                std::string tmpStr = toStr(tmp.location.reg);
                output += tmp.emitPutValue(sf->stackPos);
                output += "sub " + tmpStr + ", fp, " + tmpStr + "\n";
                output += tmp.emitCopyTo(*this);
            }
            break;
        case ExprNode::Empty:
            break;
    }
    return output;
}
