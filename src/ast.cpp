#include "ast.hpp"

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

/* SECTION: TypeNode */

TypeNode::TypeNode(BuiltinType builtinType)
        : builtinType(builtinType) {}

TypeNode::TypeNode(std::string customType)
        : customType(customType) {}

/* SECTION: ParamNode */

ParamNode::ParamNode(TypeNode *type, std::string identifier)
        : type(type),
          identifier(identifier) {}

/* SECTION: StatementNode */

StatementNode::StatementNode(TypeNode *type, std::string identifier)
        : type(type),
          identifier(identifier) {}

StatementNode::StatementNode(TypeNode *type, std::string identifier, ExprNode *expr)
        : type(type),
          identifier(identifier),
          expr(expr) {}

StatementNode::StatementNode(ExprNode *lexpr, ExprNode *rexpr)
        : lexpr(lexpr),
          expr(rexpr) {}

StatementNode::StatementNode(FnCallNode *fnCall)
        : fnCall(fnCall) {}

/* SECTION: FnCallNode */

FnCallNode::FnCallNode(ExprNode *expr, std::vector<ExprNode *> argList)
        : expr(expr), argList(argList) {}

FnCallNode::FnCallNode(ExprNode *expr)
        : expr(expr) {}

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

/* SECTION: LiteralNode */

LiteralNode::LiteralNode(LiteralType type, long val) {
    switch (type) {
        case LiteralType::Int:
            this->val.i = val & ((int)-1);
            break;
        case LiteralType::Char:
            this->val.c = val & ((char)-1);
            break;
    }
}
