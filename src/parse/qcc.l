%{
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "ast/ast.hpp"
#include "qcc.y.hpp"
#include "parse/driver.hpp"

static char _escape(char c);
static std::string _parseStr(char *str);
%}

%option noyywrap nounput noinput batch debug

IDENTIFIER [a-zA-Z_][a-zA-Z_0-9]*
INT (([0-9]+)|(0x[0-9a-f]+))
CHAR '[^\\]'
ESC_CHAR '\\.'

%{
#define YY_USER_ACTION loc.columns(yyleng);
%}

%%

%{
yy::location &loc = drv.location;
loc.step();
%}

 /* Comments */
"//".* { loc.step(); }

 /* Built-in types */
void  { return yy::parser::make_BUILTIN_TYPE(BuiltinType::Void, loc);  }
int   { return yy::parser::make_BUILTIN_TYPE(BuiltinType::Int, loc);  }
char  { return yy::parser::make_BUILTIN_TYPE(BuiltinType::Char, loc); }

 /* Keywords */
"return"   { return yy::parser::make_RETURN(loc); }
"if"       { return yy::parser::make_IF(loc); }
"else"     { return yy::parser::make_ELSE(loc); }
"while"    { return yy::parser::make_WHILE(loc); }
"break"    { return yy::parser::make_BREAK(loc); }
"continue" { return yy::parser::make_CONTINUE(loc); }

 /* Literals */
{INT}      { return yy::parser::make_INT_LITERAL(strtol(yytext, NULL, 0), loc); }
{CHAR}     { return yy::parser::make_CHAR_LITERAL(yytext[1], loc); }
{ESC_CHAR} { return yy::parser::make_CHAR_LITERAL(_escape(yytext[2]), loc); }
\"([^\\\n]|\\.)*\" { return yy::parser::make_STRING_LITERAL(_parseStr(yytext), loc); }

 /* Operators */
"+"  { return yy::parser::make_OP_PLUS   (BuiltinOperator::Plus   , loc); }
"-"  { return yy::parser::make_OP_MINUS  (BuiltinOperator::Minus  , loc); }
"*"  { return yy::parser::make_OP_STAR   (BuiltinOperator::Star   , loc); }
"/"  { return yy::parser::make_OP_FSLASH (BuiltinOperator::Fslash , loc); }
"%"  { return yy::parser::make_OP_PERCENT(BuiltinOperator::Percent, loc); }
"==" { return yy::parser::make_OP_EQ     (BuiltinOperator::Eq     , loc); }
"!=" { return yy::parser::make_OP_NE     (BuiltinOperator::Ne     , loc); }
"<"  { return yy::parser::make_OP_LT     (BuiltinOperator::Lt     , loc); }
">"  { return yy::parser::make_OP_GT     (BuiltinOperator::Gt     , loc); }
"<=" { return yy::parser::make_OP_LE     (BuiltinOperator::Le     , loc); }
">=" { return yy::parser::make_OP_GE     (BuiltinOperator::Ge     , loc); }
"!"  { return yy::parser::make_OP_NOT    (BuiltinOperator::Not    , loc); }
"~"  { return yy::parser::make_OP_BIT_NOT(BuiltinOperator::BitNot , loc); }
"&"  { return yy::parser::make_OP_BIT_AND(BuiltinOperator::BitAnd , loc); }
"|"  { return yy::parser::make_OP_BIT_OR (BuiltinOperator::BitOr  , loc); }
"^"  { return yy::parser::make_OP_BIT_XOR(BuiltinOperator::BitXor , loc); }

 /* Syntactical tokens */
"=" { return yy::parser::make_ASSIGN   (loc); }
"," { return yy::parser::make_COMMA    (loc); }
"{" { return yy::parser::make_LBRACE   (loc); }
"}" { return yy::parser::make_RBRACE   (loc); }
";" { return yy::parser::make_SEMICOLON(loc); }
"(" { return yy::parser::make_LPAREN   (loc); }
")" { return yy::parser::make_RPAREN   (loc); }
"[" { return yy::parser::make_LBRACKET (loc); }
"]" { return yy::parser::make_RBRACKET (loc); }

 /* Identifiers */
{IDENTIFIER} { return yy::parser::make_IDENTIFIER(yytext, loc); }

 /* Other characters */
[ \t\r]+ { loc.step(); }
\n+      { loc.lines(yyleng); loc.step(); }
.        { throw yy::parser::syntax_error(
            loc, "invalid character: " + std::string(yytext)); }
<<EOF>>  { return yy::parser::make_YYEOF(loc); }

%%

void Driver::scan_begin() {
    yy_flex_debug = traceScanning;
    if (file.empty() || file == "-") {
        yyin = stdin;
        return;
    }

    yyin = fopen(file.c_str(), "r");
    if (!yyin) {
        std::cerr << execName << ": "
                  << file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
    }
}

void Driver::scan_end() {
    fclose(yyin);
}

static char _escape(char c) {
    switch(c) {
        case '\\':
            return '\\';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case 'a':
            return '\a';
        case '0':
            return '\0';
        default:
            return c;
    }
}

static std::string _parseStr(char *str) {
    std::string out = "";
    while (*str) {
        out += *str == '\\'
            ? _escape(*(++str))
            : *str;
        str++;
    }
    return out.substr(1, out.length() - 2);
}
