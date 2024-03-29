#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "builtins.hpp"

class StackFrame;
class StaticData;
class CompileState;

// Declarations
class FnDeclNode;
class FnDefNode;
class TypeNode;
class ParamNode;
class StatementNode;
class IfNode;
class WhileNode;
class FnCallNode;
class ExprNode;
class LiteralNode;
class AccessorNode;

/* SECTION: Node definitions */

// Definitions

class FnDeclNode {
public:
    TypeNode *returnType;
    std::string identifier;
    std::vector <ParamNode *> paramList;
    FnDeclNode(TypeNode *returnType, std::string identifier);
    FnDeclNode(TypeNode *returnType,
               std::string identifier,
               std::vector<ParamNode *> paramList);
    virtual bool operator==(FnDeclNode &other);
    virtual bool operator!=(FnDeclNode &other);
};

class FnDefNode : public FnDeclNode {
public:
    std::vector<StatementNode *> block;
    FnDefNode(FnDeclNode fnDeclNode, std::vector<StatementNode *> block);
    void emit(CompileState &cs);
};

class TypeNode {
public:
    enum TypeKind {
        Builtin, Custom, Pointer
    } kind;
    BuiltinType builtinType;
    std::string customType;
    TypeNode *pointerType;
    TypeNode(BuiltinType builtinType);
    TypeNode(LiteralType literalType);
    TypeNode(std::string customType);
    TypeNode(TypeNode *pointerType);
    unsigned size();
    bool validOp(BuiltinOperator op, TypeNode *otherType);
    bool validOp(BuiltinOperator op);
    bool operator==(const TypeNode &other) const;
    bool operator!=(const TypeNode &other) const;
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
        Declaration, Initialization, Assignment, Return, FnCall,
        // Derived classes must be last
        If, While, Break, Continue,
    } kind;

    TypeNode *type;             // Declaration/Initialization
    std::string identifier;     // Declaration/Initialization
    AccessorNode *accessor;     // Assignment
    ExprNode *expr;             // Initialization/Assignment/Return
    FnCallNode *fnCall;         // FnCall
    std::vector<ExprNode *> *array; // ArrayInitialization

    StatementNode(TypeNode *type, std::string identifier);
    StatementNode(TypeNode *type, std::string identifier, ExprNode *expr);
    StatementNode(AccessorNode *accessor, ExprNode *rexpr);
    StatementNode(ExprNode *returnExpr);
    StatementNode(FnCallNode *fnCall);
    StatementNode(StatementKind derivedKind);

    virtual std::string emit(StackFrame *sf);
    virtual bool containsFnCalls();
    bool isDerived();
};

class IfNode : public StatementNode {
public:
    ExprNode *condition;
    std::vector<StatementNode *> block;
    std::vector<StatementNode *> elseBlock;
    IfNode(ExprNode *condition, std::vector<StatementNode *> block);
    IfNode(ExprNode *condition,
                   std::vector<StatementNode *> block,
                   std::vector<StatementNode *> elseBlock);
    virtual std::string emit(StackFrame *sf) override;
    virtual bool containsFnCalls() override;
};

class WhileNode : public StatementNode {
public:
    ExprNode *condition;
    std::vector<StatementNode *> block;
    WhileNode(ExprNode *condition, std::vector<StatementNode *> block);
    virtual std::string emit(StackFrame *sf) override;
    virtual bool containsFnCalls() override;
};

class BreakNode : public StatementNode {
public:
    BreakNode();
    virtual std::string emit(StackFrame *sf) override;
};

class ContinueNode : public StatementNode {
public:
    ContinueNode();
    virtual std::string emit(StackFrame *sf) override;
};

class FnCallNode {
public:
    std::string identifier;
    FnDeclNode *fnDecl;
    std::vector<ExprNode *> argList;
    FnCallNode(std::string identifier, FnDeclNode *fnDecl, std::vector<ExprNode *> argList);
    FnCallNode(std::string identifier, FnDeclNode *fnDecl);
private:
    void verifyParameters();
};

class ExprNode {
public:
    enum ExprKind {
        Literal, Accessor, FnCall, BinaryOp, UnaryOp, Array, Static, Empty
    } kind;

    LiteralNode *literal;               // Literal
    AccessorNode *accessor;             // Accessor
    FnCallNode *fnCall;                 // FnCall
    BuiltinOperator builtinOperator;    // BinaryOp/UnaryOp
    ExprNode *opr1, *opr2;              // BinaryOp
    ExprNode *opr;                      // UnaryOp
    std::vector<ExprNode *> *array;     // Array
    StaticData *staticData;             // Static
    TypeNode *type;

    ExprNode(LiteralNode *literal);
    ExprNode(AccessorNode *accessor);
    ExprNode(FnCallNode *fnCall);
    ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2);
    ExprNode(BuiltinOperator unaryOperator, ExprNode *opr);
    ExprNode(std::vector<ExprNode *> *array);
    ExprNode(StaticData *staticData);
    ExprNode();
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

class AccessorNode {
public:
    enum AccessorKind {
        Identifier, Dereference
    } kind;
    std::string identifier;  // Identifier
    ExprNode *expr;          // Dereference
    TypeNode *type;

    AccessorNode(std::string identifier, TypeNode *type);
    AccessorNode(ExprNode *ptr);
};

/* SECTION: Print declarations */
std::ostream &operator<<(std::ostream &os, BuiltinType &type);
std::ostream &operator<<(std::ostream &os, BuiltinOperator &op);
std::ostream &operator<<(std::ostream &os, LiteralType &type);

std::ostream &operator<<(std::ostream &os, StaticData &staticData);

std::ostream &operator<<(std::ostream &os, FnDeclNode &node);
std::ostream &operator<<(std::ostream &os, FnDefNode &node);
std::ostream &operator<<(std::ostream &os, TypeNode &node);
std::ostream &operator<<(std::ostream &os, ParamNode &node);
std::ostream &operator<<(std::ostream &os, StatementNode &node);
std::ostream &operator<<(std::ostream &os, IfNode &node);
std::ostream &operator<<(std::ostream &os, WhileNode &node);
std::ostream &operator<<(std::ostream &os, FnCallNode &node);
std::ostream &operator<<(std::ostream &os, ExprNode &node);
std::ostream &operator<<(std::ostream &os, LiteralNode &node);
std::ostream &operator<<(std::ostream &os, AccessorNode &node);

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
std::string toStr(Register res, std::string regPrefix = "x");
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
        std::string emitCopyTo(Reservation other);
        std::string emitPutValue(unsigned long val);
        std::string emitFromExprNode(StackFrame *sf, ExprNode *expr);
        bool operator==(const Reservation &other) const;
        bool operator!=(const Reservation &other) const;
    };
    std::vector<Reservation> variableReservations;
    std::vector<Reservation> exprReservations;
    std::unordered_map<std::string, Reservation> variables;

    CompileState *cs;
    FnDefNode *fnDef;
    std::vector<long> stackIncrementPadding;
    long stackPos = 0;
    long maxStackPos = 0;
    std::vector<unsigned long> loopIds;

    StackFrame(CompileState *cs, FnDefNode *fnDef);
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
    std::string emitAddressOf(Reservation res, std::string identifier);
    std::string emitSaveCaller();
    std::string emitLoadCaller();
};

class StaticData {
public:
    enum StaticDataKind {
        String, None
    } kind;
    std::string string;  // String
    unsigned long id;
    TypeNode *ptrType;

    StaticData(unsigned long id, std::string string);
    StaticData();
    std::string label();
    void emit(CompileState &cs);

private:
    unsigned p2alignment();
};

class CompileState {
public:
    std::ostream &os;
    unsigned indent = 8;
    CompileState(std::ostream &os);

    // Stack frames
    std::vector<StackFrame> frames;
    void pushFrame(FnDefNode *fnDef);
    StackFrame *getTopFrame();
    void popFrame();

    // Static data
    std::vector<StaticData *> staticData;
    StaticData *addStaticData(std::string string);
    StaticData *getStaticData(unsigned long id);

    // Keep track of which builtins to insert
    std::unordered_set<BuiltinFn> usedBuiltinFns;

    // Variable types
    std::unordered_map<std::string, TypeNode *> *varTypes;
    TypeNode *getVarType(std::string identifier);
    void setVarType(std::string identifier, TypeNode *type);

    // Function declarations/definitions
    std::unordered_map<std::string, FnDeclNode *> fnDecls;
    std::unordered_map<std::string, FnDefNode *> fnDefs;
    FnDeclNode *getFnDecl(std::string identifier);
    void addFnDecl(FnDeclNode *fnDecl);
    void addFnDef(FnDefNode *fnDef);

    // Number of if/while statements (for labeling)
    unsigned long numIfs = 0;
    unsigned long numWhiles = 0;
};
