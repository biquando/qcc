%skeleton "lalr1.cc"
%require "3.8.1"
%language "c++"

%define api.value.type variant
%define api.token.raw
%define api.token.constructor
%define api.token.prefix {TOK_}

%locations
%define parse.assert
%define parse.trace
%define parse.error detailed
%define parse.lac full

%code requires {
    #include <string>
    #include <vector>
    #include "ast/ast.hpp"
    class Driver;
}

%param {Driver &drv}
%code {
    #include "parse/driver.hpp"
    typedef std::vector<StatementNode *> Block;
}

%token ASSIGN COMMA LBRACE RBRACE SEMICOLON RETURN IF WHILE
%precedence PREC_THEN
%precedence ELSE
%left <BuiltinOperator> OP_BIT_OR
%left <BuiltinOperator> OP_BIT_XOR
%left <BuiltinOperator> OP_BIT_AND
%left <BuiltinOperator> OP_EQ OP_NE
%left <BuiltinOperator> OP_LT OP_GT OP_LE OP_GE
%left <BuiltinOperator> OP_PLUS OP_MINUS
%left <BuiltinOperator> OP_STAR OP_FSLASH
%precedence <BuiltinOperator> OP_NOT OP_BIT_NOT
%left LPAREN RPAREN

%token <BuiltinType> BUILTIN_TYPE
%token <std::string> IDENTIFIER
%token <long> INT_LITERAL
%token <char> CHAR_LITERAL
%token <std::string> STRING_LITERAL

%type <FnDeclNode *> fnDecl fnSignature
%type <FnDefNode *> fnDef
%type <TypeNode *> type
%type <ParamNode *> param
%type <StatementNode *> statement declaration initialization assignment return
%type <IfNode *> if
%type <WhileNode *> while
%type <FnCallNode *> fnCall
%type <ExprNode *> expr
%type <LiteralNode *> literal

%type <std::vector<ParamNode *> *> paramList
%type <std::vector<StatementNode *> *> block blockWithBraces statementBlock
%type <std::vector<ExprNode *> *> argList
%type <ExprNode *> array

%%

%start file;
file
    :
    | file fnDecl {
        delete drv.cs->varTypes;
        drv.cs->varTypes = new std::unordered_map<std::string, TypeNode *>();
    }
    | file fnDef {
        drv.fnDefNodes.push_back($2);
        delete drv.cs->varTypes;
        drv.cs->varTypes = new std::unordered_map<std::string, TypeNode *>();
      }
    ;

fnDecl
    : fnSignature SEMICOLON { $$ = $1; }
    ;

fnSignature
    : type IDENTIFIER LPAREN RPAREN {
        $$ = new FnDeclNode($1, $2);
        drv.cs->addFnDecl($$);
      }
    | type IDENTIFIER LPAREN paramList RPAREN {
        $$ = new FnDeclNode($1, $2, *$4);
        drv.cs->addFnDecl($$);
        delete $4;
      }
    ;

fnDef
    : fnSignature blockWithBraces {
        $$ = new FnDefNode(*$1, *$2);
        drv.cs->addFnDef($$);
        delete $2;
      }
    ;

type
    : BUILTIN_TYPE { $$ = new TypeNode($1); }
    /* | IDENTIFIER { $$ = new TypeNode($1); } */
    ;

paramList
    : param { $$ = new std::vector<ParamNode *>(); $$->push_back($1); }
    | paramList COMMA param { $1->push_back($3); $$ = $1; }
    ;

param
    : type IDENTIFIER { $$ = new ParamNode($1, $2); drv.cs->setVarType($2, $1); }
    ;

blockWithBraces
    : LBRACE block RBRACE { $$ = $2; }
    ;

block
    : { $$ = new std::vector<StatementNode *>(); }
    | block statement { $1->push_back($2); $$ = $1; }
    ;

statement
    : declaration SEMICOLON { $$ = $1; }
    | initialization SEMICOLON { $$ = $1; }
    | assignment SEMICOLON { $$ = $1; }
    | return SEMICOLON { $$ = $1; }
    | fnCall SEMICOLON { $$ = new StatementNode($1); }
    | if { $$ = $1; }
    | while { $$ = $1; }
    ;

declaration
    : type IDENTIFIER { $$ = new StatementNode($1, $2); drv.cs->setVarType($2, $1); }
    ;

initialization
    : type IDENTIFIER ASSIGN expr { $$ = new StatementNode($1, $2, $4); drv.cs->setVarType($2, $1); }
    | type IDENTIFIER ASSIGN array { $$ = new StatementNode($1, $2, $4); drv.cs->setVarType($2, $1); }
    ;

assignment
    : IDENTIFIER ASSIGN expr { $$ = new StatementNode($1, $3); }
    ;

return
    : RETURN expr { $$ = new StatementNode($2); }
    | RETURN      { $$ = new StatementNode(new ExprNode()); }
    ;

fnCall
    : IDENTIFIER LPAREN argList RPAREN { $$ = new FnCallNode($1, drv.cs->getFnDecl($1), *$3); delete $3; }
    | IDENTIFIER LPAREN RPAREN { $$ = new FnCallNode($1, drv.cs->getFnDecl($1)); }
    ;

if
    : IF LPAREN expr RPAREN statementBlock {
        $$ = new IfNode($3, *$5);
        delete $5;
      } %prec PREC_THEN
    | IF LPAREN expr RPAREN statementBlock ELSE statementBlock {
        $$ = new IfNode($3, *$5, *$7);
        delete $5;
        delete $7;
      }
    ;

while
    : WHILE LPAREN expr RPAREN statementBlock {
        $$ = new WhileNode($3, *$5);
        delete $5;
      }

statementBlock
    : statement { $$ = new std::vector<StatementNode *>{$1}; }
    | blockWithBraces { $$ = $1; }
    ;

argList
    : expr { $$ = new std::vector<ExprNode *>(); $$->push_back($1); }
    | argList COMMA expr { $1->push_back($3); $$ = $1; }
    ;

array
    : LBRACE RBRACE { $$ = new ExprNode(new std::vector<ExprNode *>()); }
    | LBRACE argList RBRACE { $$ = new ExprNode($2); }
    | STRING_LITERAL {
        auto *arr = new std::vector<ExprNode *>();
        for (char c : $1) {
            ExprNode *charExpr = new ExprNode(new LiteralNode(c));
            arr->push_back(charExpr);
        }
        $$ = new ExprNode(arr);
    }
    ;

expr
    : literal { $$ = new ExprNode($1); }
    | IDENTIFIER { $$ = new ExprNode($1, drv.cs->getVarType($1)); }
    | fnCall { $$ = new ExprNode($1); }
    | expr OP_PLUS    expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_MINUS   expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_STAR    expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_FSLASH  expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_EQ      expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_NE      expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_LT      expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_GT      expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_LE      expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_GE      expr { $$ = new ExprNode($2, $1, $3); }
    | OP_MINUS        expr { $$ = new ExprNode($1, $2); }
    | OP_NOT          expr { $$ = new ExprNode($1, $2); }
    | OP_BIT_NOT      expr { $$ = new ExprNode($1, $2); }
    | expr OP_BIT_AND expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_BIT_OR  expr { $$ = new ExprNode($2, $1, $3); }
    | expr OP_BIT_XOR expr { $$ = new ExprNode($2, $1, $3); }
    | OP_BIT_AND IDENTIFIER {
        $$ = new ExprNode($1, new ExprNode($2, drv.cs->getVarType($2)));
    }
    | OP_STAR         expr { $$ = new ExprNode($1, $2); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

literal
    : INT_LITERAL { $$ = new LiteralNode($1); }
    | CHAR_LITERAL { $$ = new LiteralNode($1); }
    ;

%%

void yy::parser::error(const location_type &l, const std::string &str) {
    std::cerr << l << ": " << str << '\n';
    drv.res = 1;
}
