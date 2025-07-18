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
    CompoundAssignmentStatementType, NegativeExpressionType
};

class Statement {
public:
    SigmaAstType type;
    Statement(SigmaAstType t): type(t) {};

    virtual ~Statement() {};
};

class SigmaProgram : public Statement {
public:
    std::vector<std::shared_ptr<Statement>> stmts;
    SigmaProgram (std::vector<std::shared_ptr<Statement>> statements): Statement(ProgramType),
        stmts(std::move(statements)) {};
};

class Expression : public Statement {
public:
    Expression(SigmaAstType expr_type): Statement(expr_type) {};
};

class BinaryExpression : public Expression {
public:
    std::shared_ptr<Expression> left;
    std::shared_ptr<Expression> right;
    std::string op;

    BinaryExpression(std::shared_ptr<Expression> l, std::shared_ptr<Expression> r, std::string oper):
        Expression(BinaryExpressionType), left(l), right(r), op(oper) {};
};

class StringExpression : public Expression {
public:
    std::string str;

    StringExpression(std::string stri): Expression(StringExpressionType), str(stri) {};
};

class IdentifierExpression : public Expression {
public:
    std::string str;

    IdentifierExpression(std::string stri): Expression(IdentifierExpressionType), str(stri) {};
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
    std::vector<std::shared_ptr<Expression>> args;

    StructExpression(std::string name, std::vector<std::shared_ptr<Expression>> arguments):
        Expression(StructExpressionType), struct_name(name), args(arguments) {};
};

class VariableDecleration : public Expression {
public:
    std::string var_name;
    std::shared_ptr<Expression> expr;
    bool is_const;

    VariableDecleration(std::string name, std::shared_ptr<Expression> val, bool is_constant):
        Expression(VariableDeclerationType), var_name(name), expr(val), is_const(is_constant) {};
};

class VariableReInit : public Expression {
public:
    std::string var_name;
    std::shared_ptr<Expression> expr;

    VariableReInit(std::string name, std::shared_ptr<Expression> val):
        Expression(VariableReInitializationType), var_name(name), expr(val) {};
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
    std::vector<std::shared_ptr<Statement>> stmts;

    ElseStatement(std::vector<std::shared_ptr<Statement>> statements):
        Statement(ElseStatementType), stmts(statements) {};
};

class ElseIfStatement : public Statement {
public:
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<Statement>> stmts;

    ElseIfStatement(std::shared_ptr<Expression> expression, 
        std::vector<std::shared_ptr<Statement>> statements): Statement(ElseIfStatementType),
        expr(expression), stmts(statements) {};
};

class IfStatement : public Statement {
public:
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<Statement>> stmts;
    std::vector<std::shared_ptr<ElseIfStatement>> else_if_stmts;
    std::shared_ptr<ElseStatement> else_stmt;

    IfStatement(std::shared_ptr<Expression> expression, 
        std::vector<std::shared_ptr<Statement>> statements,std::vector<std::shared_ptr<ElseIfStatement>>
         else_if_stmts_arg, std::shared_ptr<ElseStatement> else_stmt_arg): Statement(IfStatementType),
        expr(expression), stmts(statements), else_if_stmts(else_if_stmts_arg),
        else_stmt(else_stmt_arg) {};
};

class WhileLoopStatement : public Statement {
public:
    std::shared_ptr<Expression> expr;
    std::vector<std::shared_ptr<Statement>> stmts;

    WhileLoopStatement(std::shared_ptr<Expression> expression, 
        std::vector<std::shared_ptr<Statement>> statements): Statement(WhileStatementType),
        expr(expression), stmts(statements) {};
};

class ForLoopStatement : public Statement {
public:
    std::shared_ptr<Statement> first_stmt;
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Statement> last_stmt;
    std::vector<std::shared_ptr<Statement>> stmts;

    ForLoopStatement(std::shared_ptr<Expression> expression, 
        std::vector<std::shared_ptr<Statement>> statements,
    std::shared_ptr<Statement> first_statement, std::shared_ptr<Statement> last_statement)
    : Statement(ForStatementType),
    expr(expression), stmts(statements), first_stmt(first_statement), last_stmt(last_statement) {};
};

class ArrayExpression : public Expression {
public:
    std::vector<std::shared_ptr<Expression>> exprs;

    ArrayExpression(std::vector<std::shared_ptr<Expression>> expressions): Expression(ArrayExpressionType),
        exprs(expressions) {};
};

class LambdaExpression : public Expression {
public:
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Statement>> stmts;

    LambdaExpression(std::vector<std::string> parameters,
        std::vector<std::shared_ptr<Statement>> statements):
        Expression(LambdaExpressionType), params(parameters), stmts(statements) {};
};

class FunctionCallExpression : public Expression {
public:
    std::shared_ptr<Expression> func_expr;
    std::vector<std::shared_ptr<Expression>> args;

    FunctionCallExpression(std::shared_ptr<Expression> expr, std::vector<std::shared_ptr<Expression>> arguments):
        Expression(FunctionCallExpressionType), func_expr(expr), args(arguments) {};
};

class IndexAccessExpression : public Expression {
public:
    std::shared_ptr<Expression> array_expr;
    std::vector<std::shared_ptr<Expression>> path;

    IndexAccessExpression(std::shared_ptr<Expression> array_expression,
        std::vector<std::shared_ptr<Expression>> access_path): Expression(IndexAccessExpressionType),
        array_expr(array_expression), path(access_path) {};
};

class IndexReInitStatement : public Statement {
public:
    std::shared_ptr<Expression> array_expr;
    std::vector<std::shared_ptr<Expression>> path;
    std::shared_ptr<Expression> val;

    IndexReInitStatement(std::shared_ptr<Expression> array_expression,
        std::vector<std::shared_ptr<Expression>> access_path,
        std::shared_ptr<Expression> value): Statement(IndexReInitStatementType),
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
    std::shared_ptr<Expression> expr;
    ReturnStatement(std::shared_ptr<Expression> expression): Statement(ReturnStatementType),
        expr(expression) {};
};

class StructDeclerationStatement : public Statement {
public:
    std::string struct_name;
    std::vector<std::shared_ptr<VariableDecleration>> props;

    StructDeclerationStatement(std::string struc_name, std::vector<std::shared_ptr<VariableDecleration>> properties):
        Statement(StructDeclerationType), struct_name(struc_name), props(properties) {};
};

class MemberAccessExpression : public Expression {
public:
    std::shared_ptr<Expression> struct_expr;
    std::vector<std::string> path;

    MemberAccessExpression(std::shared_ptr<Expression> expr,
        std::vector<std::string> p): Expression(MemberAccessExpressionType), struct_expr(expr),
        path(p) {};
};

class MemberReInitExpression : public Expression {
public:
    std::shared_ptr<Expression> struct_expr;
    std::vector<std::string> path;
    std::shared_ptr<Expression> val;

    MemberReInitExpression(std::shared_ptr<Expression> expr, std::vector<std::string> vec, 
        std::shared_ptr<Expression> v): Expression(MemberReInitExpressionType),
        struct_expr(expr), path(vec), val(v) {};
};

class IncrementExpression : public Expression {
public:
    std::shared_ptr<Expression> expr;
    double amount;

    IncrementExpression(std::shared_ptr<Expression> actual_expr,
        double amou): Expression(IncrementExpressionType),
        expr(actual_expr), amount(amou) {};
};

class DecrementExpression : public Expression {
public:
    std::shared_ptr<Expression> expr;

    DecrementExpression(std::shared_ptr<Expression> actual_expr):
        Expression(DecrementExpressionType),
        expr(actual_expr) {};
};

class CompoundAssignmentStatement : public Statement {
public:
    std::shared_ptr<Expression> expr;
    std::shared_ptr<Expression> amount;
    std::string oper;

    CompoundAssignmentStatement(std::shared_ptr<Expression> exprr, std::string op,
        std::shared_ptr<Expression> actual_amount): Statement(CompoundAssignmentStatementType),
        expr(exprr), amount(actual_amount), oper(op) {};
};

class NegativeExpression : public Expression {
public:
    std::shared_ptr<Expression> expr;

    NegativeExpression(std::shared_ptr<Expression> actual_expr): Expression(NegativeExpressionType),
        expr(actual_expr) {};
};