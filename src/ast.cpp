#include "ast.hpp"
#include "util.hpp"

/* SECTION: Enums */

std::ostream &operator<<(std::ostream &os, BuiltinType &type) {
    switch (type) {
        case BuiltinType::Void: return os << "Void";
        case BuiltinType::Int:  return os << "Int";
        case BuiltinType::Char: return os << "Char";
    }
}

std::ostream &operator<<(std::ostream &os, BuiltinOperator &op) {
    switch (op) {
        case BuiltinOperator::Plus:   return os << "Plus";
        case BuiltinOperator::Minus:  return os << "Minus";
        case BuiltinOperator::Star:   return os << "Star";
        case BuiltinOperator::Fslash: return os << "Fslash";
        case BuiltinOperator::Eq:     return os << "Eq";
        case BuiltinOperator::Ne:     return os << "Ne";
        case BuiltinOperator::Lt:     return os << "Lt";
        case BuiltinOperator::Gt:     return os << "Gt";
        case BuiltinOperator::Le:     return os << "Le";
        case BuiltinOperator::Ge:     return os << "Ge";
        case BuiltinOperator::Not:    return os << "Not";
    }
}

std::ostream &operator<<(std::ostream &os, LiteralType &type) {
    switch (type) {
        case LiteralType::Int:  return os << "Int";
        case LiteralType::Char: return os << "Char";
    }
}

/* SECTION: FnDefNode */

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

/* SECTION: TypeNode */

TypeNode::TypeNode(BuiltinType builtinType)
        : builtinType(builtinType), isCustom(false) {}

TypeNode::TypeNode(std::string customType)
        : customType(customType), isCustom(true) {}

std::ostream &operator<<(std::ostream &os, TypeNode &node) {
    os << "TypeNode: ";
    if (node.isCustom) {
        os << node.customType;
    } else {
        os << node.builtinType;
    }
    return os;
}

/* SECTION: ParamNode */

ParamNode::ParamNode(TypeNode *type, std::string identifier)
        : type(type),
          identifier(identifier) {}

std::ostream &operator<<(std::ostream &os, ParamNode &node) {
    os << "ParamNode: (";
    os << *(node.type) << ") " << node.identifier;
    return os;
}

/* SECTION: StatementNode */

StatementNode::StatementNode(TypeNode *type, std::string identifier)
        : kind(Declaration),
          type(type),
          identifier(identifier) {}

StatementNode::StatementNode(TypeNode *type, std::string identifier, ExprNode *expr)
        : kind(Initialization),
          type(type),
          identifier(identifier),
          expr(expr) {}

StatementNode::StatementNode(ExprNode *lexpr, ExprNode *rexpr)
        : kind(Assignment),
          lexpr(lexpr),
          expr(rexpr) {}

StatementNode::StatementNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall) {}

std::ostream &operator<<(std::ostream &os, StatementNode &node) {
    IndentedStream ios(os);
    os << "StatementNode (";
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
            os << "Assignment):";
            ios << '\n' << *(node.lexpr);
            ios << '\n' << *(node.expr);
            break;
        case StatementNode::FnCall:
            os << "FnCall):";
            ios << '\n' << *(node.fnCall);
            break;
    }
    return os;
}

/* SECTION: FnCallNode */

FnCallNode::FnCallNode(ExprNode *expr, std::vector<ExprNode *> argList)
        : expr(expr), argList(argList) {}

FnCallNode::FnCallNode(ExprNode *expr)
        : expr(expr) {}

std::ostream &operator<<(std::ostream &os, FnCallNode &node) {
    IndentedStream ios(os);
    os << "FnCallNode: (" << *(node.expr) << ") (";
    if (!node.argList.empty()) { os << '\n'; }
    for (auto &arg : node.argList) {
        ios << *arg << '\n';
    }
    return os << ')';
}

/* SECTION: ExprNode */

ExprNode::ExprNode(LiteralNode *literal)
        : kind(Literal),
          literal(literal) {}

ExprNode::ExprNode(std::string identifier)
        : kind(Identifier),
          identifier(identifier) {}

ExprNode::ExprNode(FnCallNode *fnCall)
        : kind(FnCall),
          fnCall(fnCall) {}

ExprNode::ExprNode(BuiltinOperator binaryOperator, ExprNode *opr1, ExprNode *opr2)
        : kind(BinaryOp),
          builtinOperator(binaryOperator),
          opr1(opr1),
          opr2(opr2) {}

ExprNode::ExprNode(BuiltinOperator unaryOperator, ExprNode *opr)
        : kind(UnaryOp),
          builtinOperator(unaryOperator),
          opr(opr) {}

std::ostream &operator<<(std::ostream &os, ExprNode &node) {
    IndentedStream ios(os);
    os << "ExprNode";
    switch (node.kind) {
        case ExprNode::Literal:
            os << " (" << *(node.literal) << ')';
            break;
        case ExprNode::Identifier:
            os << " (Identifier: " << node.identifier << ')';
            break;
        case ExprNode::FnCall:
            os << ": " << *(node.fnCall);
            break;
        case ExprNode::BinaryOp:
            os << " (BinaryOp: " << node.builtinOperator << ")\n";
            ios << *(node.opr1) << '\n';
            ios << *(node.opr2) << '\n';
            break;
        case ExprNode::UnaryOp:
            os << " (UnaryOp: " << node.builtinOperator << ")\n";
            ios << *(node.opr) << '\n';
            break;
    }
    return os;
}

/* SECTION: LiteralNode */

LiteralNode::LiteralNode(int i)
        : type(LiteralType::Int),
          i(i) {}

LiteralNode::LiteralNode(char c)
        : type(LiteralType::Char),
          c(c) {}

std::ostream &operator<<(std::ostream &os, LiteralNode &node) {
    os << "LiteralNode ";
    switch (node.type) {
        case LiteralType::Int:
            os << node.i;
            break;
        case LiteralType::Char:
            os << '\'' << node.c << '\'';
            break;
    }
    return os;
}
