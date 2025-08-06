#pragma once
#include "SigmaAst.h"
#include "SigmaLexer.h"
#include <memory>
#include <memory_resource>

typedef Statement* Stmt;
typedef Expression* Expr;

class SigmaParser {
public:
    static std::pmr::unsynchronized_pool_resource memory_pool;
    SigmaProgram* produceAst(std::vector<SigmaToken> tokens);

    template<typename ValType, typename ...ArgsType>
    static ValType* makeAst(ArgsType... args) {
        void* mem = memory_pool.allocate(sizeof(ValType), alignof(ValType));
        ValType* obj = new(mem)ValType(std::forward<ArgsType>(args)...);

        return obj;
    };
    template<typename ValType>
    static void freeAst(ValType** ptr){
        memory_pool.deallocate(*ptr, sizeof(ValType), alignof(ValType));
        *ptr = nullptr;
    }
    
    Stmt parseStmt();
    Stmt parseVarDeclStmt();
    Stmt parseVarReInitStmt();
    Stmt parseIfStmt();
    Stmt parseWhileLoop();
    Stmt parseForLoop();
    Stmt parseStructDeclerationStmt();
    Stmt parseIndexReInitStmt(Expr arr);
    Stmt parseMemberReInitStmt(Expr stru);
    Stmt parseCompoundAssignmentStmt(Expr targ_expr);
    Stmt parseFunctionDecl();

    Expr parseExpr();
    Expr parseAddExpr();
    Expr parseMulExpr();
    Expr parseCompExpr();
    Expr parseBitWiseExpr();
    Expr parsePrimaryExpr();
    Expr parseLambdaExpr();
    Expr parseFunctionCall(Expr expr);
    Expr parseArrayExpr();
    Expr parseIndexExpr(Expr arr);
    Expr parseStructExpr();
    Expr parseJsObjectExpr();
    Expr parseMemberAccessExpr(Expr struc);
    

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