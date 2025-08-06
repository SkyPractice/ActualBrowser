#include "SigmaParser.h"
#include "SigmaAst.h"
#include "SigmaLexer.h"
#include <iostream>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <unordered_map>
#include <utility>

std::pmr::unsynchronized_pool_resource SigmaParser::memory_pool = std::pmr::unsynchronized_pool_resource();

MemberAccessExpression* MemberAccessExpression::clone_expr(){
    return SigmaParser::makeAst<MemberAccessExpression>(struct_expr, path);
}


SigmaProgram* SigmaParser::produceAst(std::vector<SigmaToken> tokens) {
    std::vector<Stmt> stmts;
    itr = tokens.begin();

    while(itr->type != ENDOFFILETOK)
        stmts.push_back(parseStmt());

    return makeAst<SigmaProgram>(stmts);
};

Stmt SigmaParser::parseStmt() {
    if(itr->type == Identifier && std::next(itr)->type == Equal)
        return parseVarReInitStmt();
    switch (itr->type) {
        case Var:
        case Const:
            return parseVarDeclStmt();
        case Function:
            return parseFunctionDecl();
        case If:
            return parseIfStmt();
        case While:
            return parseWhileLoop();
        case For:
            return parseForLoop();
        case Continue:
            advance();
            return makeAst<ContinueStatement>();
        case Break:
            advance();
            return makeAst<BreakStatement>();
        case Return:{
            advance();
            Expr expr = parseExpr();
            return makeAst<ReturnStatement>(expr);
        }break;
        case Struct:
            return parseStructDeclerationStmt();
        default: { 
            Expr expr = parseExpr();
            if(expr->type == IndexAccessExpressionType && itr->type == Equal){
                auto expression = dynamic_cast<IndexAccessExpression*>(expr);
                advance(); // eat equal
                auto express = parseExpr();
                return makeAst<IndexReInitStatement>(expression->array_expr,
                    expression->path, express);
            } else if (expr->type == MemberAccessExpressionType && itr->type == Equal){
                auto expression = dynamic_cast<MemberAccessExpression*>(expr);
                advance(); // eat equal
                auto express = parseExpr();
                return makeAst<MemberReInitExpression>(expression->struct_expr,
                    expression->path, express);
            } else if (itr->type == CompoundAssignmentOperator){
                return parseCompoundAssignmentStmt(expr);
            }
            return expr;
        }break;
    }

};
Stmt SigmaParser::parseFunctionDecl() {
    const SigmaToken func = advance();
    const std::string name = advance().symbol;

    const SigmaToken open_paren = advance();
    std::vector<std::string> strs;

    while(itr->type != CloseParen){
        strs.push_back(advance().symbol);
        if(itr->type == Comma) advance();
        else break;
    }

    const SigmaToken close_paren = advance();
    const SigmaToken open_brace = advance();

    std::vector<Stmt> stmts;
    while(itr->type != CloseBrace)
        stmts.push_back(parseStmt());
    advance();

    LambdaExpression* lambda_expr = makeAst<LambdaExpression>(strs, stmts);

    return makeAst<VariableDecleration>(name, lambda_expr, true);
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

    return makeAst<VariableDecleration>(name, expr, is_const);
};
Stmt SigmaParser::parseVarReInitStmt() {
    const std::string var_name = advance().symbol;
    const SigmaToken eq = advance();
    const Expr expr = parseExpr();

    return makeAst<VariableReInit>(var_name, expr);
};

Expr SigmaParser::parseExpr() { 
    auto expr = parseAddExpr();


    return expr;
};
Expr SigmaParser::parseAddExpr() {
    Expr left = parseMulExpr();

    while(itr->symbol == "+" || itr->symbol == "-"){
        std::string op = advance().symbol;
        left = makeAst<BinaryExpression>(left, parseMulExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseMulExpr() {
    Expr left = parseCompExpr();

    while(itr->symbol == "*" || itr->symbol == "/" || itr->symbol == "%"){
        std::string op = advance().symbol;
        left = makeAst<BinaryExpression>(left, parseCompExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseCompExpr() {
    Expr left = parseBitWiseExpr();

    while(itr->symbol == "==" || itr->symbol == ">=" || itr->symbol == "<=" ||
        itr->symbol == "!=" || itr->symbol == ">" || itr->symbol == "<"){
        std::string op = advance().symbol;
        left = makeAst<BinaryExpression>(left, parseBitWiseExpr(), op);
    }

    return left;
};
Expr SigmaParser::parseBitWiseExpr() {
    Expr left = parsePrimaryExpr();
    while (itr->type == OpenBracket || itr->type == Dot || itr->type == OpenParen){
        if(itr->type == OpenBracket){
            left = parseIndexExpr(left);
        }
        else if(itr->type == Dot){
            left = parseMemberAccessExpr(left);
        }
        else if (itr->type == OpenParen){
            left = parseFunctionCall(left);
        }
    }

    while(itr->symbol == "&" || itr->symbol == "|" || itr->symbol == "<<" ||
        itr->symbol == ">>" || itr->symbol == "^"){
        std::string op = advance().symbol;
        auto r = parsePrimaryExpr();
        while (itr->type == OpenBracket || itr->type == Dot || itr->type == OpenParen){
            if(itr->type == OpenBracket){
                r = parseIndexExpr(r);
            }
            else if(itr->type == Dot){
                r = parseMemberAccessExpr(r);
            }
            else if (itr->type == OpenParen){
                r = parseFunctionCall(r);
            }
    }
        left = makeAst<BinaryExpression>(left, r, op);
    }

    return left;
};
Expr SigmaParser::parsePrimaryExpr() {
    if(itr->symbol == "-"){
        advance();
        return makeAst<NegativeExpression>(parseExpr());
    }
    switch (itr->type) {
        case Number:
            return makeAst<NumericExpression>(std::stod(advance().symbol));
        case Str:{
            return makeAst<StringExpression>(advance().symbol);
        }break;
        case Char: {
            return makeAst<CharExpression>(advance().symbol[0]);
        }break;
        case True: advance(); return makeAst<BoolExpression>(true);
        case False: advance(); return makeAst<BoolExpression>(false);
        case OpenParen: {
            auto iterat = itr;
            iterat++;
            while(iterat->type != CloseParen)
                iterat++;

            if(std::next(iterat)->type == LambdaIndicator){
                return parseLambdaExpr();
            }
            advance();
            Expr expr = parseExpr();
            advance();
            return expr;
        }break;
        case Identifier:{
            return makeAst<IdentifierExpression>(advance().symbol);
        }break;
        case OpenBracket:
            return parseArrayExpr();
        case New:
            return parseStructExpr();
        case OpenBrace:
            return parseJsObjectExpr();
        case Increment:
            advance();
            return makeAst<IncrementExpression>(parseExpr(), 1);
        case Decrement:
            advance();
            return makeAst<IncrementExpression>(parseExpr() , -1);
        case Null:
            advance();
            return makeAst<NullExpression>();
        case Negative:
        default: throw std::runtime_error("Expression Type Not Implemented " + std::to_string(itr->type));
    }
};
Expr SigmaParser::parseLambdaExpr() {
    const SigmaToken open_paren = advance();
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
    return makeAst<LambdaExpression>(strs, stmts);
};
Expr SigmaParser::parseFunctionCall(Expr expr) {
    const SigmaToken open_paren = advance();
    std::vector<Expr> exprs;

    while(itr->type != CloseParen){
        exprs.push_back(parseExpr());
        if(itr->type == Comma) advance();
        else break;
    }

    const SigmaToken close_paren = advance();

    return makeAst<FunctionCallExpression>(expr, exprs);
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

    return makeAst<ArrayExpression>(exprs);
}

Expr SigmaParser::parseIndexExpr(Expr arr) {
    std::vector<Expr> path;
    while(itr->type == OpenBracket){
        advance();
        path.push_back(parseExpr());
        advance();
    }

    return makeAst<IndexAccessExpression>(arr, path);
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

    return makeAst<IndexReInitStatement>(arr, path, val);
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

    std::vector<ElseIfStatement*> else_if_stmts;
    while(itr->type == ElseIf){
        advance(); // eat elseif
        Expr expr = parseExpr();
        advance(); // through the {
        std::vector<Stmt> statements;
        while (itr->type != CloseBrace) {
            statements.push_back(parseStmt());
        }
        advance();

        else_if_stmts.push_back(makeAst<ElseIfStatement>(expr, statements));
    }
    ElseStatement* else_stmt = nullptr;

    if(itr->type == Else){
        advance(); // through the else
        advance(); // through the {
        std::vector<Stmt> stmtss;
        while(itr->type != CloseBrace){
            stmtss.push_back(parseStmt());
        }
        advance();

        else_stmt = makeAst<ElseStatement>(stmtss);
    }

    return makeAst<IfStatement>(expr, stmts, else_if_stmts, else_stmt);
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

    return makeAst<WhileLoopStatement>(expr, stmts);
};
Stmt SigmaParser::parseForLoop() {
    advance(); // eat for
    Stmt first_stmt = nullptr;
    Expr expr = nullptr;
    Stmt last_stmt = nullptr;
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

    return makeAst<ForLoopStatement>(expr, stmts, first_stmt, last_stmt);
};

Stmt SigmaParser::parseStructDeclerationStmt(){
    advance(); // through "struct"
    std::string struct_iden = advance().symbol;
    advance(); // through {
    std::vector<VariableDecleration*> propss;
    while(itr->type != CloseBrace){
        propss.push_back(dynamic_cast<VariableDecleration*>(parseStmt()));
        if(itr->type == Comma)
            advance();
        else break;
    }
    advance(); // through }
    
    return makeAst<StructDeclerationStatement>(struct_iden, propss);
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

    return makeAst<StructExpression>(stru_name, exprs);
};

Expr SigmaParser::parseMemberAccessExpr(Expr struc) {
    std::vector<std::string> path;

    while(itr->type == Dot){
        advance();
        path.push_back(advance().symbol);
    }

    return makeAst<MemberAccessExpression>(struc, path);
};

Stmt SigmaParser::parseCompoundAssignmentStmt(Expr targ_expr) {
    std::string op = advance().symbol;
    Expr val = parseExpr();
    return makeAst<CompoundAssignmentStatement>(targ_expr, op, val);
};

Expr SigmaParser::parseJsObjectExpr() {
    advance(); // through the {

    std::vector<std::pair<std::string, Expr>> exprs;

    while(itr->type != CloseBrace){
        std::string iden = advance().symbol;
        advance(); // through the :
        Expr expr = parseExpr();
        exprs.push_back({iden, expr});
        if(itr->type == Comma)
            advance();
        else break;
    }
    advance();

    return makeAst<JsObjectExpression>(std::move(exprs));
};