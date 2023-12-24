#pragma once

#include <string>
#include <vector>

enum class BuiltinType {
    Int,
    Char,
};

enum class BuiltinOperator {
    Plus, Minus, Star, Fslash,
    Eq, Ne,
    Lt, Gt, Le, Ge,
    Not,
};

enum class LiteralType {
    Int,
    Char,
};

/* SECTION: Node definitions */

// Declarations
class FnDefNode;
class TypeNode;
class ParamNode;
class StatementNode;
class FnCallNode;
class ExprNode;
class LiteralNode;

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
};

class TypeNode {
public:
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
        Declaration, Initialization, Assignment, FnCall
    } kind;

    TypeNode *type;             // Declaration
    std::string identifier;     // Declaration/Initialization
    ExprNode *lexpr, *expr;     // Initialization/Assignment
    FnCallNode *fnCall;         // FnCall

    StatementNode(TypeNode *type, std::string identifier);
    StatementNode(TypeNode *type, std::string identifier, ExprNode *expr);
    StatementNode(ExprNode *lexpr, ExprNode *rexpr);
    StatementNode(FnCallNode *fnCall);
};

class FnCallNode {
public:
    ExprNode *expr;
    std::vector<ExprNode *> argList;
    FnCallNode(ExprNode *expr, std::vector<ExprNode *> argList);
    FnCallNode(ExprNode *expr);
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
};

class LiteralNode {
public:
    LiteralType type;
    int i;
    char c;

    LiteralNode(int i);
    LiteralNode(char c);
};

