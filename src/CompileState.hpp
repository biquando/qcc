#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class CompileState;

enum class BuiltinType {
    Void,
    Int,
    Char,
};

enum class BuiltinOperator {
    Plus, Minus, Star, Fslash,
    Eq, Ne,
    Lt, Gt, Le, Ge,
    Not,
    BitNot, BitAnd, BitOr, BitXor
};

enum class LiteralType {
    Int,
    Char,
};

// Declarations
class FnDefNode;
class TypeNode;
class ParamNode;
class StatementNode;
class FnCallNode;
class ExprNode;
class LiteralNode;

/* SECTION: Node definitions */

// Definitions

class FnDefNode {
public:
    TypeNode *returnType;
    std::string identifier;
    std::vector<ParamNode *> paramList;
    std::vector<StatementNode *> block;

    FnDefNode(TypeNode *returnType,
              std::string identifier,
              std::vector<StatementNode *> block);
    FnDefNode(TypeNode *returnType,
              std::string identifier,
              std::vector<ParamNode *> paramList,
              std::vector<StatementNode *> block);

    void emit(CompileState &cs);
};

class TypeNode {
public:
    bool isCustom;
    BuiltinType builtinType;
    std::string customType;
    TypeNode(BuiltinType builtinType);
    TypeNode(std::string customType);
};

class ParamNode {
public:
    TypeNode *type;
    std::string identifier;
    ParamNode(TypeNode *type, std::string identifier);
};

class StatementNode {
public:
    enum StatementKind {
        Declaration, Initialization, Assignment, Return, FnCall
    } kind;

    TypeNode *type;             // Declaration/Initialization
    std::string identifier;     // Declaration/Initialization/Assignment
    ExprNode *expr;             // Initialization/Assignment/Return
    FnCallNode *fnCall;         // FnCall

    StatementNode(TypeNode *type, std::string identifier);
    StatementNode(TypeNode *type, std::string identifier, ExprNode *expr);
    StatementNode(std::string identifier, ExprNode *rexpr);
    StatementNode(ExprNode *returnExpr);
    StatementNode(FnCallNode *fnCall);
    bool containsFnCalls();
};

class FnCallNode {
public:
    std::string identifier;
    std::vector<ExprNode *> argList;
    FnCallNode(std::string identifier, std::vector<ExprNode *> argList);
    FnCallNode(std::string identifier);
};

class ExprNode {
public:
    enum ExprKind {
        Literal, Identifier, FnCall, BinaryOp, UnaryOp,
    } kind;

    LiteralNode *literal;               // Literal
    std::string identifier;             // Identifier
    FnCallNode *fnCall;                 // FnCall
    BuiltinOperator builtinOperator;    // BinaryOp/UnaryOp
    ExprNode *opr1, *opr2;              // BinaryOp
    ExprNode *opr;                      // UnaryOp

    ExprNode(LiteralNode *literal);
    ExprNode(std::string identifier);
    ExprNode(FnCallNode *fnCall);
    ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2);
    ExprNode(BuiltinOperator unaryOperator, ExprNode *opr);
    bool containsFnCalls();
};

class LiteralNode {
public:
    LiteralType type;
    long i;
    char c;

    LiteralNode(long i);
    LiteralNode(char c);
};

/* SECTION: Print declarations */
std::ostream &operator<<(std::ostream &os, BuiltinType &type);
std::ostream &operator<<(std::ostream &os, BuiltinOperator &op);
std::ostream &operator<<(std::ostream &os, LiteralType &type);

std::ostream &operator<<(std::ostream &os, FnDefNode &node);
std::ostream &operator<<(std::ostream &os, TypeNode &node);
std::ostream &operator<<(std::ostream &os, ParamNode &node);
std::ostream &operator<<(std::ostream &os, StatementNode &node);
std::ostream &operator<<(std::ostream &os, FnCallNode &node);
std::ostream &operator<<(std::ostream &os, ExprNode &node);
std::ostream &operator<<(std::ostream &os, LiteralNode &node);

/*
  Register Conventions:
    x0-x7    procedure arguments
    x8       indirect return value address
    x9-x15   caller-saved (local variables)
    x16-x17  intra-procedure-call scratch register
    x18      platform register (don't use)
    x19-x28  callee-saved
    x29      fp
    x30      lr
    x31      sp
*/
enum class Register {
    x0 = 0, x1, x2, x3, x4, x5, x6, x7,
    x8,
    x9, x10, x11, x12, x13, x14, x15,
    x16, ip0 = x16, x17, ip1 = x17,
    x18, pr = x18,
    x19, x20, x21, x22, x23, x24, x25, x26, x27, x28,
    x29, fp = x29,
    x30, lr = x30,
    x31, sp = x31,
};
std::string toStr(Register res);
std::string toStr(long l);
std::ostream &operator<<(std::ostream &os, Register &reg);

class StackFrame {
public:
    struct Reservation {
        union {
            Register reg;
            long stackOffset;
        } location;
        enum {
            Reg, Stack,
        } kind;
        TypeNode *type;
        bool valid = true;

        Reservation(TypeNode *type, Register reg);
        Reservation(TypeNode *type, long stackOffset);
        Reservation();
        bool operator==(Reservation &other);
        std::string emitCopyTo(Reservation other);
        std::string emitPutValue(unsigned long val);
        std::string emitFromExprNode(StackFrame *sf, ExprNode *expr);
    };
    std::vector<Reservation> variableReservations;
    std::vector<Reservation> exprReservations;
    std::unordered_map<std::string, Reservation> variables;

    long stackPos = 0;
    long maxStackPos = 0;

    StackFrame();
    void incStackPos(long amt);

    void addVariable(TypeNode *type, std::string identifier);
    Reservation getVariable(std::string identifier);

    Reservation reserveVariable(TypeNode *type);
    Reservation reserveExpr(TypeNode *type);
    void unreserveVariable();
    void unreserveExpr();
    std::string emitBinaryOp(BuiltinOperator op, Reservation res,
                             Reservation opr1, Reservation opr2);
    std::string emitUnaryOp(BuiltinOperator op, Reservation res,
                            Reservation opr);
    std::string emitSaveCaller();
    std::string emitLoadCaller();
};

class CompileState {
public:
    std::ostream &os;
    unsigned indent = 8;
    CompileState(std::ostream &os);

    std::vector<StackFrame> frames;
    void pushFrame();
    StackFrame *getTopFrame();
    void popFrame();
};
