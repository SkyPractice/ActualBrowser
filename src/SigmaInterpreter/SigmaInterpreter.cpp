#include "SigmaInterpreter.h"
#include "RunTime.h"
#include "SigmaAst.h"
#include <algorithm>
#include <memory>
#include <stdexcept>

SigmaInterpreter::SigmaInterpreter(){
    native_functions.insert({"println", &SigmaInterpreter::println});
    native_functions.insert({ "toString", &SigmaInterpreter::toString });
};

RunTimeValue SigmaInterpreter::evaluate(Stmt stmt) {
    switch (stmt->type) {
        case NumericExpressionType:
            return std::make_shared<NumVal>(std::dynamic_pointer_cast<NumericExpression>(stmt)->num);
        case StringExpressionType:
            return std::make_shared<StringVal>(std::dynamic_pointer_cast<StringExpression>(stmt)->str);
        case BooleanExpressionType:
            return std::make_shared<BoolVal>(std::dynamic_pointer_cast<BoolExpression>(stmt)->val);
        case LambdaExpressionType:{
            auto stm = std::dynamic_pointer_cast<LambdaExpression>(stmt);
            return std::make_shared<LambdaVal>(stm->params, stm->stmts);
        }
        case ArrayExpressionType:{
            auto stm = std::dynamic_pointer_cast<ArrayExpression>(stmt);
            std::vector<RunTimeValue> vals(stm->exprs.size());
            std::transform(stm->exprs.begin(), stm->exprs.end(), vals.begin(),
                [this](Expr expr){ return evaluate(expr); });
            return std::make_shared<ArrayVal>(vals);
        }
        case BinaryExpressionType:
            return evaluateBinaryExpression(std::dynamic_pointer_cast<BinaryExpression>(stmt));
        case ProgramType:
            return evaluateProgram(std::dynamic_pointer_cast<SigmaProgram>(stmt));    
        case VariableDeclerationType:
            return evaluateVariableDeclStatement(std::dynamic_pointer_cast<VariableDecleration>(stmt));
        case VariableReInitializationType:
            return evaluateVariableReInitStatement(std::dynamic_pointer_cast<VariableReInit>(stmt));
        case FunctionCallExpressionType:
            return evaluateFunctionCallExpression(std::dynamic_pointer_cast<FunctionCallExpression>(stmt));
        case IdentifierExpressionType:
            return current_scope->getVal(std::dynamic_pointer_cast<IdentifierExpression>(stmt)->str);
        default: throw std::runtime_error("Not Implemented " + std::to_string(stmt->type));
    }
};

RunTimeValue SigmaInterpreter::evaluateProgram(std::shared_ptr<SigmaProgram> program) {
    current_scope = std::make_shared<Scope>(nullptr);

    for(auto& stmt : program->stmts){
        evaluate(stmt);
    }

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateBinaryExpression(std::shared_ptr<BinaryExpression> expr) {
    auto left = evaluate(expr);
    auto right = evaluate(expr);

    if(left->type == NumType && right->type == NumType){
        auto l = std::dynamic_pointer_cast<NumVal>(left);
        auto r = std::dynamic_pointer_cast<NumVal>(right);

        return evaluateNumericBinaryExpression(l, r, expr->op);
    }
    if(left->type == StringType && right->type == StringType){
        auto l = std::dynamic_pointer_cast<StringVal>(left);
        auto r = std::dynamic_pointer_cast<StringVal>(right);

        return evaluateStringBinaryExpression(l, r, expr->op);
    }
    if(left->type == BoolType && right->type == BoolType){
        auto l = std::dynamic_pointer_cast<BoolVal>(left);
        auto r = std::dynamic_pointer_cast<BoolVal>(right);

        return evaluateBooleanBinaryExpression(l, r, expr->op);
    }

    throw std::runtime_error("Binary Operators Not Implemented For Operands " + 
        std::to_string((int)left->type) + "," + std::to_string((int)right->type));
};

RunTimeValue SigmaInterpreter::evaluateVariableDeclStatement(std::shared_ptr<VariableDecleration> decl) {
    current_scope->declareVar(decl->var_name, { evaluate(decl->expr),
         decl->is_const });
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateVariableReInitStatement(std::shared_ptr<VariableReInit> decl) {
    current_scope->reInitVar(decl->var_name, evaluate(decl->expr));
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateFunctionCallExpression(std::shared_ptr<FunctionCallExpression> expr) {
    if(native_functions.contains(expr->func_name)){
        std::vector<RunTimeValue> args(expr->args.size());

        std::transform(expr->args.begin(), expr->args.end(), args.begin(),
            [this](Expr& expr){ return evaluate(expr); });
        
        return native_functions[expr->func_name](args);
    } else {
        // not implemented
        return nullptr;
    }
};

RunTimeValue SigmaInterpreter::evaluateNumericBinaryExpression(std::shared_ptr<NumVal> left,
    std::shared_ptr<NumVal> right, std::string operat) {
    if(operat == "+")
        return std::make_shared<NumVal>(left->num + right->num);
    if(operat == "-")
        return std::make_shared<NumVal>(left->num - right->num);
    if(operat == "*")
        return std::make_shared<NumVal>(left->num * right->num);
    if(operat == "/")
        return std::make_shared<NumVal>(left->num / right->num);
    if(operat == "%")
        return std::make_shared<NumVal>((int)left->num % (int)right->num);
    if(operat == "&")
        return std::make_shared<NumVal>((int)left->num & (int)right->num);
    if(operat == "|")
        return std::make_shared<NumVal>((int)left->num | (int)right->num);
    if(operat == ">>")
        return std::make_shared<NumVal>((int)left->num >> (int)right->num);
    if(operat == "<<")
        return std::make_shared<NumVal>((int)left->num << (int)right->num);
    if(operat == "==")
        return std::make_shared<BoolVal>(left->num == right->num);
    if(operat == ">")
        return std::make_shared<BoolVal>(left->num > right->num);
    if(operat == "<")
        return std::make_shared<BoolVal>(left->num < right->num);
    if(operat == ">=")
        return std::make_shared<BoolVal>(left->num >= right->num);
    if(operat == "<=")
        return std::make_shared<BoolVal>(left->num <= right->num);
    if(operat == "!=")
        return std::make_shared<BoolVal>(left->num != right->num);

    throw std::runtime_error("operator " + operat + " isn't valid between operands Number, Number");
};
RunTimeValue SigmaInterpreter::evaluateBooleanBinaryExpression(std::shared_ptr<BoolVal> left,
    std::shared_ptr<BoolVal> right, std::string op) {
    if(op == "==")
        return std::make_shared<BoolVal>(left->boolean == right->boolean);
    if(op == "!=")
        return std::make_shared<BoolVal>(left->boolean != right->boolean);
    if(op == ">")
        return std::make_shared<BoolVal>(left->boolean > right->boolean);
    if(op == "<")
        return std::make_shared<BoolVal>(left->boolean < right->boolean);
    if(op == ">=")
        return std::make_shared<BoolVal>(left->boolean >= right->boolean);
    if(op == "<=")
        return std::make_shared<BoolVal>(left->boolean <= right->boolean);
    if(op == "|")
        return std::make_shared<NumVal>(left->boolean | right->boolean);
    if(op == "&")
        return std::make_shared<NumVal>(left->boolean & right->boolean);
    if(op == "&&")
        return std::make_shared<BoolVal>(left->boolean && right->boolean);
    if(op == "||")
        return std::make_shared<BoolVal>(left->boolean || right->boolean);
    if(op == ">>")
        return std::make_shared<NumVal>(left->boolean >> right->boolean);
    if(op == "<<")
        return std::make_shared<NumVal>(left->boolean << right->boolean);

    throw std::runtime_error("operator " + op + " isn't valid between operands Boolean, Boolean");
};
// deprecated
RunTimeValue SigmaInterpreter::evaluateStringBinaryExpression(std::shared_ptr<StringVal> left,
    std::shared_ptr<StringVal> right, std::string op) {
    if(op == "==")
        return std::make_shared<BoolVal>(left->str == right->str);
    if(op == "!=")
        return std::make_shared<BoolVal>(left->str != right->str);
    if(op == ">")
        return std::make_shared<BoolVal>(left->str > right->str);
    if(op == "<")
        return std::make_shared<BoolVal>(left->str < right->str);
    if(op == ">=")
        return std::make_shared<BoolVal>(left->str >= right->str);
    if(op == "<=")
        return std::make_shared<BoolVal>(left->str <= right->str);

    throw std::runtime_error("operator " + op + " isn't valid between operands String, String");
};