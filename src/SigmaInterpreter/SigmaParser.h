#pragma once
#include "SigmaAst.h"
#include "SigmaLexer.h"
#include <memory>

typedef std::shared_ptr<Statement> Stmt;
typedef std::shared_ptr<Expression> Expr;

class SigmaParser {
public:
    std::shared_ptr<SigmaProgram> produceAst(std::vector<SigmaToken> tokens);
    
    Stmt parseStmt();
    Stmt parseVarDeclStmt();
    Stmt parseVarReInitStmt();
    Stmt parseIfStmt();
    Stmt parseWhileLoop();
    Stmt parseForLoop();
    Stmt parseStructDeclerationStmt();
    Stmt parseIndexReInitStmt(Expr arr);

    Expr parseExpr();
    Expr parseAddExpr();
    Expr parseMulExpr();
    Expr parseCompExpr();
    Expr parseBitWiseExpr();
    Expr parsePrimaryExpr();
    Expr parseLambdaExpr();
    Expr parseFunctionCall();
    Expr parseArrayExpr();
    Expr parseIndexExpr(Expr arr);
    Expr parseStructExpr();


    std::vector<SigmaToken>::iterator itr;

    SigmaToken advance(){
        if(itr->type != ENDOFFILETOK){
            SigmaToken tok = *itr;
            itr++;
            return tok;
        }
        return *itr;
    }
};