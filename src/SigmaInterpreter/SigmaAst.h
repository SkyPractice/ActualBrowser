#pragma once
#include <memory>
#include <string>
#include <vector>

enum SigmaAstType {
    StatementType, ProgramType, ExpressionType, BinaryExpressionType, StringExpressionType,
    NumericExpressionType, BooleanExpressionType, StructExpressionType, LambdaExpressionType,
    VariableDeclerationType, VariableReInitializationType, EnumDeclerationType, ClassDeclerationType,
    StructDeclerationType, IncludeType, IfStatementType, ElseIfStatementType, ElseStatementType,
    WhileStatementType, ForStatementType, ArrayExpressionType, IndexAccessExpressionType,
    IndexReInitStatementType,
    MemberAccessExpressionType, PropertyDeclerationType, NameSpaceType, IdentifierExpressionType,
    FunctionCallExpressionType, ContinueStatementType, ReturnStatementType, BreakStatementType,
    MemberReInitExpressionType, IncrementExpressionType, DecrementExpressionType, 
    CompoundAssignmentStatementType, NegativeExpressionType,
    JsObjectExprType, CharExpressionType, NullExpressionType
};

class Statement {
public:
    SigmaAstType type;
    Statement(SigmaAstType t): type(t) {};

    virtual ~Statement() {};
};

class SigmaProgram : public Statement {
public:
    std::vector<Statement*> stmts;
    SigmaProgram (std::vector<Statement*> statements): Statement(ProgramType),
        stmts(std::move(statements)) {};
};

class Expression : public Statement {
public:
    Expression(SigmaAstType expr_type): Statement(expr_type) {};
};

class BinaryExpression : public Expression {
public:
    Expression* left;
    Expression* right;
    std::string op;

    BinaryExpression(Expression* l, Expression* r, std::string oper):
        Expression(BinaryExpressionType), left(l), right(r), op(oper) {};
};

class StringExpression : public Expression {
public:
    std::string str;

    StringExpression(std::string stri): Expression(StringExpressionType), str(std::move(stri)) {};
};

class NullExpression : public Expression {
public:
    NullExpression(): Expression(NullExpressionType) {};
};

class CharExpression : public Expression {
public:
    char character;

    CharExpression(char ch): Expression(CharExpressionType), character(ch) {};
};

class IdentifierExpression : public Expression {
public:
    std::string str;

    IdentifierExpression(std::string stri): Expression(IdentifierExpressionType), str(std::move(stri)) {};
};

class NumericExpression : public Expression {
public:
    double num;

    NumericExpression(double number): Expression(NumericExpressionType), num(number) {};
};

class BoolExpression : public Expression {
public:
    bool val;

    BoolExpression(bool value): Expression(BooleanExpressionType), val(value) {};
};

class StructExpression : public Expression {
public:
    std::string struct_name;
    std::vector<Expression*> args;

    StructExpression(std::string name, std::vector<Expression*> arguments):
        Expression(StructExpressionType), struct_name(name), args(arguments) {};
};

class VariableDecleration : public Expression {
public:
    std::string var_name;
    Expression* expr;
    bool is_const;

    VariableDecleration(std::string name, Expression* val, bool is_constant):
        Expression(VariableDeclerationType), var_name(name), expr(val), is_const(is_constant) {};
};

class VariableReInit : public Expression {
public:
    std::string var_name;
    Expression* expr;

    VariableReInit(std::string name, Expression* val):
        Expression(VariableReInitializationType), var_name(std::move(name)), expr(val) {};
};

class EnumDecleration : public Statement {
public:
    std::vector<std::string> values;

    EnumDecleration(std::vector<std::string> vals): Statement(EnumDeclerationType),
        values(vals) {};
};

class IncludeStatement : public Statement {
public:
    std::string file_path;

    IncludeStatement(std::string path): Statement(IncludeType), file_path(path) {};
};

class ElseStatement : public Statement {
public:
    std::vector<Statement*> stmts;

    ElseStatement(std::vector<Statement*> statements):
        Statement(ElseStatementType), stmts(statements) {};
};

class ElseIfStatement : public Statement {
public:
    Expression* expr;
    std::vector<Statement*> stmts;

    ElseIfStatement(Expression* expression, 
        std::vector<Statement*> statements): Statement(ElseIfStatementType),
        expr(expression), stmts(statements) {};
};

class IfStatement : public Statement {
public:
    Expression* expr;
    std::vector<Statement*> stmts;
    std::vector<ElseIfStatement*> else_if_stmts;
    ElseStatement* else_stmt;

    IfStatement(Expression* expression, 
        std::vector<Statement*> statements,std::vector<ElseIfStatement*>
         else_if_stmts_arg, ElseStatement* else_stmt_arg): Statement(IfStatementType),
        expr(expression), stmts(statements), else_if_stmts(else_if_stmts_arg),
        else_stmt(else_stmt_arg) {};
};

class WhileLoopStatement : public Statement {
public:
    Expression* expr;
    std::vector<Statement*> stmts;

    WhileLoopStatement(Expression* expression, 
        std::vector<Statement*> statements): Statement(WhileStatementType),
        expr(expression), stmts(statements) {};
};

class ForLoopStatement : public Statement {
public:
    Statement* first_stmt;
    Expression* expr;
    Statement* last_stmt;
    std::vector<Statement*> stmts;

    ForLoopStatement(Expression* expression, 
        std::vector<Statement*> statements,
    Statement* first_statement, Statement* last_statement)
    : Statement(ForStatementType),
    expr(expression), stmts(statements), first_stmt(first_statement), last_stmt(last_statement) {};
};

class ArrayExpression : public Expression {
public:
    std::vector<Expression*> exprs;

    ArrayExpression(std::vector<Expression*> expressions): Expression(ArrayExpressionType),
        exprs(expressions) {};
};

class LambdaExpression : public Expression {
public:
    std::vector<std::string> params;
    std::vector<Statement*> stmts;

    LambdaExpression(std::vector<std::string> parameters,
        std::vector<Statement*> statements):
        Expression(LambdaExpressionType), params(parameters), stmts(statements) {};
};

class FunctionCallExpression : public Expression {
public:
    Expression* func_expr;
    std::vector<Expression*> args;

    FunctionCallExpression(Expression* expr, std::vector<Expression*> arguments):
        Expression(FunctionCallExpressionType), func_expr(expr), args(arguments) {};
};

class IndexAccessExpression : public Expression {
public:
    Expression* array_expr;
    std::vector<Expression*> path;

    IndexAccessExpression(Expression* array_expression,
        std::vector<Expression*> access_path): Expression(IndexAccessExpressionType),
        array_expr(array_expression), path(access_path) {};
};

class IndexReInitStatement : public Statement {
public:
    Expression* array_expr;
    std::vector<Expression*> path;
    Expression* val;

    IndexReInitStatement(Expression* array_expression,
        std::vector<Expression*> access_path,
        Expression* value): Statement(IndexReInitStatementType),
        array_expr(array_expression), path(access_path), val(value) {};
};

class ContinueStatement : public Statement {
public:
    ContinueStatement(): Statement(ContinueStatementType) {};
};

class BreakStatement : public Statement {
public:
    BreakStatement(): Statement(BreakStatementType) {};
};

class ReturnStatement : public Statement {
public:
    Expression* expr;
    ReturnStatement(Expression* expression): Statement(ReturnStatementType),
        expr(expression) {};
};

class StructDeclerationStatement : public Statement {
public:
    std::string struct_name;
    std::vector<VariableDecleration*> props;

    StructDeclerationStatement(std::string struc_name, std::vector<VariableDecleration*> properties):
        Statement(StructDeclerationType), struct_name(struc_name), props(properties) {};
};

class MemberAccessExpression : public Expression {
public:
    Expression* struct_expr;
    std::vector<std::string> path;

    MemberAccessExpression(Expression* expr,
        std::vector<std::string> p): Expression(MemberAccessExpressionType), struct_expr(expr),
        path(p) {};
    MemberAccessExpression* clone_expr();
};

class MemberReInitExpression : public Expression {
public:
    Expression* struct_expr;
    std::vector<std::string> path;
    Expression* val;

    MemberReInitExpression(Expression* expr, std::vector<std::string> vec, 
        Expression* v): Expression(MemberReInitExpressionType),
        struct_expr(expr), path(vec), val(v) {};
};

class IncrementExpression : public Expression {
public:
    Expression* expr;
    VariableReInit* cached_variable_reinit = nullptr;
    MemberReInitExpression* cached_member_reinit = nullptr;
    IndexReInitStatement* cached_index_reinit = nullptr;
    double amount;

    IncrementExpression(Expression* actual_expr,
        double amou): Expression(IncrementExpressionType),
        expr(actual_expr), amount(amou) {};
};

class DecrementExpression : public Expression {
public:
    Expression* expr;

    DecrementExpression(Expression* actual_expr):
        Expression(DecrementExpressionType),
        expr(actual_expr) {};
};

class CompoundAssignmentStatement : public Statement {
public:
    Expression* expr;
    Expression* amount;
    std::string oper;

    CompoundAssignmentStatement(Expression* exprr, std::string op,
        Expression* actual_amount): Statement(CompoundAssignmentStatementType),
        expr(exprr), amount(actual_amount), oper(op) {};
};

class NegativeExpression : public Expression {
public:
    Expression* expr;

    NegativeExpression(Expression* actual_expr): Expression(NegativeExpressionType),
        expr(actual_expr) {};
};

class JsObjectExpression : public Expression {
public:
    std::vector<std::pair<std::string, Expression*>> exprs;

    JsObjectExpression(std::vector<std::pair<std::string,
        Expression*>> expressions): Expression(JsObjectExprType), exprs(expressions) {};
};