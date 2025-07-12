#pragma once
#include "RunTime.h"
#include "SigmaAst.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

typedef std::shared_ptr<Statement> Stmt;
typedef std::shared_ptr<Expression> Expr;
typedef std::shared_ptr<RunTimeVal> RunTimeValue;

class Scope : public std::enable_shared_from_this<Scope>{
public: 
    std::shared_ptr<Scope> parent;
    std::unordered_map<std::string, RunTimeValue> variables;

    Scope(std::shared_ptr<Scope> p): parent(p) {};

    std::shared_ptr<Scope> traverse(std::string var_name){
        if(variables.contains(var_name)) return shared_from_this();
        else if (parent) return parent->traverse(var_name);
        return nullptr;
    };

    RunTimeValue getVal(std::string var_name){
        auto scope = traverse(var_name);
        if(!scope){ throw std::runtime_error("variable " + var_name + " not found"); };
        return scope->variables[var_name];
    };

    // shadowing is allowed
    void declareVar(std::string name, RunTimeValue val){
        variables[name] = val;
    };

    void reInitVar(std::string name, RunTimeValue val){
        auto scope = traverse(name);
        if(!scope) { throw std::runtime_error("variable " + name + " not found"); };
        scope->variables[name] = val;
    }
};

class SigmaInterpreter {
public:
    std::shared_ptr<Scope> current_scope = std::make_shared<Scope>(nullptr);
    std::unordered_map<std::string, std::function<RunTimeValue(std::vector<RunTimeValue>)>> 
        native_functions;

    SigmaInterpreter();
    
    RunTimeValue evaluate(Stmt stmt);
    RunTimeValue evaluateProgram(std::shared_ptr<SigmaProgram> program);
    RunTimeValue evaluateBinaryExpression(std::shared_ptr<BinaryExpression> expr);
    RunTimeValue evaluateNumericBinaryExpression(std::shared_ptr<NumVal> left,
        std::shared_ptr<NumVal> right, std::string operat);
    RunTimeValue evaluateBooleanBinaryExpression(std::shared_ptr<BoolVal> left,
        std::shared_ptr<BoolVal> right, std::string op);
    RunTimeValue evaluateStringBinaryExpression(std::shared_ptr<StringVal> left,
        std::shared_ptr<StringVal> right, std::string op);

    static RunTimeValue println(std::vector<RunTimeValue> args);
    static RunTimeValue toString(std::vector<RunTimeValue> args);
    static RunTimeValue numIota(std::vector<RunTimeValue> args);
};