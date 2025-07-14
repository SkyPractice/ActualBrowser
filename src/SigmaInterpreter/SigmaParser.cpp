#include "SigmaParser.h"
#include "SigmaAst.h"
#include "SigmaLexer.h"
#include <iostream>
#include <memory>
#include <stdexcept>

std::shared_ptr<SigmaProgram> SigmaParser::produceAst(std::vector<SigmaToken> tokens) {
    std::vector<Stmt> stmts;
    itr = tokens.begin();

    while(itr->type != ENDOFFILETOK)
        stmts.push_back(parseStmt());

    return std::make_shared<SigmaProgram>(stmts);
};

Stmt SigmaParser::parseStmt() {
    if(itr->type == Identifier && std::next(itr)->type == Equal)
        return parseVarReInitStmt();
    switch (itr->type) {
        case Var:
        case Const:
            return parseVarDeclStmt();
        case If:
            return parseIfStmt();
        case While:
            return parseWhileLoop();
        case For:
            return parseForLoop();
        case Continue:
            advance();
            return std::make_shared<ContinueStatement>();
        case Break:
            advance();
            return std::make_shared<BreakStatement>();
        case Return:{
            advance();
            Expr expr = parseExpr();
            return std::make_shared<ReturnStatement>(expr);
        }break;
        case Struct:
            return parseStructDeclerationStmt();
        default: { 
            Expr expr = parseExpr();
            if(expr->type == IndexAccessExpressionType && itr->type == Equal){
                auto expression = std::dynamic_pointer_cast<IndexAccessExpression>(expr);
                advance(); // eat equal
                auto express = parseExpr();
                return std::make_shared<IndexReInitStatement>(expression->array_expr,
                    expression->path, express);
            } else if (expr->type == MemberAccessExpressionType && itr->type == Equal){
                auto expression = std::dynamic_pointer_cast<MemberAccessExpression>(expr);
                advance(); // eat equal
                auto express = parseExpr();
                return std::make_shared<MemberReInitExpression>(expression->struct_expr,
                    expression->path, express);
            }
            return expr;
        }break;
    }

};
Stmt SigmaParser::parseVarDeclStmt() {
    const SigmaToken word = advance();
    const std::string name = advance().symbol;
    const SigmaToken equal = *itr;
    const bool is_const = !(word.type == Var);
    Expr expr = nullptr;

    if(equal.type == Equal){
        advance();
        expr = parseExpr();
    } else if(is_const) throw std::runtime_error("A Const Variable Must Be Initialized");

    return std::make_shared<VariableDecleration>(name, expr, is_const);
};
Stmt SigmaParser::parseVarReInitStmt() {
    const std::string var_name = advance().symbol;
    const SigmaToken eq = advance();
    const Expr expr = parseExpr();

    return std::make_shared<VariableReInit>(var_name, expr);
};

Expr SigmaParser::parseExpr() { 
    auto expr = parseAddExpr();
    while (itr->type == OpenBracket || itr->type == Dot){
        if(itr->type == OpenBracket){
            expr = parseIndexExpr(expr);
        }
        else if(itr->type == Dot){
            expr = parseMemberAccessExpr(expr);
        }
    }

    return expr;
};
Expr SigmaParser::parseAddExpr() {
    Expr left = parseMulExpr();

    while(itr->symbol == "+" || itr->symbol == "-"){
        std::string op = advance().symbol;
        left = std::make_shared<BinaryExpression>(left, parseMulExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseMulExpr() {
    Expr left = parseCompExpr();

    while(itr->symbol == "*" || itr->symbol == "/" || itr->symbol == "%"){
        std::string op = advance().symbol;
        left = std::make_shared<BinaryExpression>(left, parseCompExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseCompExpr() {
    Expr left = parseBitWiseExpr();

    while(itr->symbol == "==" || itr->symbol == ">=" || itr->symbol == "<=" ||
        itr->symbol == "!=" || itr->symbol == ">" || itr->symbol == "<"){
        std::string op = advance().symbol;
        left = std::make_shared<BinaryExpression>(left, parseBitWiseExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseBitWiseExpr() {
    Expr left = parsePrimaryExpr();

    while(itr->symbol == "&" || itr->symbol == "|" || itr->symbol == "<<" ||
        itr->symbol == ">>" || itr->symbol == "^"){
        std::string op = advance().symbol;
        left = std::make_shared<BinaryExpression>(left, parsePrimaryExpr(), op);
    }

    return left;
};
Expr SigmaParser::parsePrimaryExpr() {
    switch (itr->type) {
        case Number:
            return std::make_shared<NumericExpression>(std::stod(advance().symbol));
        case Str:{
            return std::make_shared<StringExpression>(advance().symbol);
        }break;
        case True: advance(); return std::make_shared<BoolExpression>(true);
        case False: advance(); return std::make_shared<BoolExpression>(false);
        case OpenParen: {
            auto iterat = itr;
            iterat++;
            while(iterat->type != CloseParen)
                iterat++;

            if(std::next(iterat)->symbol == "=>"){
                return parseLambdaExpr();
            }
            advance();
            Expr expr = parseExpr();
            advance();
            return expr;
        }break;
        case Identifier:{
            if(std::next(itr)->type == OpenParen)
                return parseFunctionCall();
            return std::make_shared<IdentifierExpression>(advance().symbol);
        }break;
        case OpenBracket:
            return parseArrayExpr();
        case New:
            return parseStructExpr();
        default: throw std::runtime_error("Expression Type Not Implemented " + std::to_string(itr->type));
    }
};
Expr SigmaParser::parseLambdaExpr() {
    const SigmaToken open_paren = advance();
    std::cout << open_paren.symbol << " paren";
    std::vector<std::string> strs;

    while(itr->type != CloseParen){
        strs.push_back(advance().symbol);
        if(itr->type == Comma) advance();
        else break;
    }

    const SigmaToken close_paren = advance();
    const SigmaToken lambda_arrow = advance(); // =>
    const SigmaToken open_brace = advance();

    std::vector<Stmt> stmts;
    while(itr->type != CloseBrace)
        stmts.push_back(parseStmt());
    advance();
    return std::make_shared<LambdaExpression>(strs, stmts);
};
Expr SigmaParser::parseFunctionCall() {
    const std::string func_name = advance().symbol;
    const SigmaToken open_paren = advance();
    std::vector<Expr> exprs;

    std::cout << itr->type << " incorrect itr type" << std::endl;
    while(itr->type != CloseParen){
        exprs.push_back(parseExpr());
        if(itr->type == Comma) advance();
        else break;
    }

    const SigmaToken close_paren = advance();

    return std::make_shared<FunctionCallExpression>(func_name, exprs);
};

Expr SigmaParser::parseArrayExpr() {
    std::vector<Expr> exprs;
    advance();
    while(itr->type != CloseBracket){
        exprs.push_back(parseExpr());
        if(itr->type == Comma) advance();
        else break;
    }
    advance();

    return std::make_shared<ArrayExpression>(exprs);
}

Expr SigmaParser::parseIndexExpr(Expr arr) {
    std::vector<Expr> path;
    while(itr->type == OpenBracket){
        advance();
        path.push_back(parseExpr());
        advance();
    }

    return std::make_shared<IndexAccessExpression>(arr, path);
}

Stmt SigmaParser::parseIndexReInitStmt(Expr arr) {
    std::vector<Expr> path;
    while(itr->type == OpenBracket){
        advance();
        path.push_back(parseExpr());
        advance();
    }
    advance(); // eat equal
    Expr val = parseExpr();

    return std::make_shared<IndexReInitStatement>(arr, path, val);
}

Stmt SigmaParser::parseIfStmt() {
    advance(); // eat if
    Expr expr = parseExpr();
    advance(); // through the {
    std::vector<Stmt> stmts;
    while(itr->type != CloseBrace){
        stmts.push_back(parseStmt());
    }
    advance();

    std::vector<std::shared_ptr<ElseIfStatement>> else_if_stmts;
    while(itr->type == ElseIf){
        advance(); // eat elseif
        Expr expr = parseExpr();
        advance(); // through the {
        std::vector<Stmt> statements;
        while (itr->type != CloseBrace) {
            statements.push_back(parseStmt());
        }
        advance();

        else_if_stmts.push_back(std::make_shared<ElseIfStatement>(expr, statements));
    }
    std::shared_ptr<ElseStatement> else_stmt;

    if(itr->type == Else){
        advance(); // through the else
        advance(); // through the {
        std::vector<Stmt> stmtss;
        while(itr->type != CloseBrace){
            stmtss.push_back(parseStmt());
        }
        advance();

        else_stmt = std::make_shared<ElseStatement>(stmtss);
    }

    return std::make_shared<IfStatement>(expr, stmts, else_if_stmts, else_stmt);
};
Stmt SigmaParser::parseWhileLoop() {
    advance(); // eat while
    Expr expr = parseExpr();
    advance(); // through the {
    std::vector<Stmt> stmts;
    while(itr->type != CloseBrace){
        stmts.push_back(parseStmt());
    }
    advance();

    return std::make_shared<WhileLoopStatement>(expr, stmts);
};
Stmt SigmaParser::parseForLoop() {
    advance(); // eat for
    Stmt first_stmt;
    Expr expr;
    Stmt last_stmt;
    if(itr->type != Comma){
        first_stmt = parseStmt();
        advance();
    } else advance();
    if(itr->type != Comma){
        expr = parseExpr();
        advance();
    } else advance();
    if(itr->type != OpenBrace){
        last_stmt = parseStmt();
    }
    
    advance(); // through the {
    std::vector<Stmt> stmts;
    while(itr->type != CloseBrace){
        stmts.push_back(parseStmt());
    }
    advance();

    return std::make_shared<ForLoopStatement>(expr, stmts, first_stmt, last_stmt);
};

Stmt SigmaParser::parseStructDeclerationStmt(){
    advance(); // through "struct"
    std::string struct_iden = advance().symbol;
    advance(); // through {
    std::vector<std::string> propss;
    while(itr->type != CloseBrace){
        propss.push_back(advance().symbol);
        if(itr->type == Comma)
            advance();
        else break;
    }
    advance(); // through }
    
    return std::make_shared<StructDeclerationStatement>(struct_iden, propss);
};

Expr SigmaParser::parseStructExpr(){
    advance(); // through "new"
    const std::string stru_name = advance().symbol;
    const SigmaToken open_paren = advance();
    std::vector<Expr> exprs;

    while(itr->type != CloseParen){
        exprs.push_back(parseExpr());
        if(itr->type == Comma) advance();
        else break;
    }

    const SigmaToken close_paren = advance();

    return std::make_shared<StructExpression>(stru_name, exprs);
};

Expr SigmaParser::parseMemberAccessExpr(Expr struc) {
    std::vector<std::string> path;

    while(itr->type == Dot){
        advance();
        path.push_back(advance().symbol);
    }

    return std::make_shared<MemberAccessExpression>(struc, path);
};