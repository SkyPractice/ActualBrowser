#include "SigmaInterpreter.h"
#include "RunTime.h"
#include "SigmaAst.h"
#include <algorithm>
#include <iostream>
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
        case IfStatementType:
            return evaluateIfStatement(std::dynamic_pointer_cast<IfStatement>(stmt));
        case WhileStatementType:
            return evaluateWhileLoopStatement(std::dynamic_pointer_cast<WhileLoopStatement>(stmt));
        case ForStatementType:
            return evaluateForLoopStatement(std::dynamic_pointer_cast<ForLoopStatement>(stmt));
        case ContinueStatementType:
            return std::make_shared<ContinueVal>();
        case BreakStatementType:
            return std::make_shared<BreakVal>();
        case ReturnStatementType:
            return std::make_shared<ReturnVal>(
                evaluate(std::dynamic_pointer_cast<ReturnStatement>(stmt)->expr));
        default: throw std::runtime_error("Not Implemented " + std::to_string(stmt->type));
    }
};

RunTimeValue SigmaInterpreter::evaluateIfStatement(std::shared_ptr<IfStatement> if_stmt) {
    auto expr = evaluate(if_stmt->expr);
    if(expr->type != BoolType)
        throw std::runtime_error("if statement expression must result in a boolean value");
    if(std::dynamic_pointer_cast<BoolVal>(expr)->boolean){
        auto scope = std::make_shared<Scope>(current_scope);
        current_scope = scope;

        for(auto& stmt : if_stmt->stmts){
            auto result = evaluate(stmt);
            if(!result) continue;
            if(break_out_types.contains(result->type)){
                current_scope = current_scope->parent;
                return result;
            }
        }

        current_scope = current_scope->parent;
    } else {
        for(auto& else_if_stmt : if_stmt->else_if_stmts){
            auto else_if_expr = evaluate(if_stmt->expr);

            if(else_if_expr->type != BoolType){
                throw std::runtime_error("elseif statement expression must result in a boolean value"); 
            } 
        
            if(std::dynamic_pointer_cast<BoolVal>(else_if_expr)->boolean){
                auto scope = std::make_shared<Scope>(current_scope);
                current_scope = scope;

                for(auto& stmt : else_if_stmt->stmts){
                    auto result = evaluate(stmt);
                    if(!result) continue;
                    if (break_out_types.contains(result->type)) {
                      current_scope = current_scope->parent;
                      return result;
                    }
                }

                current_scope = current_scope->parent;
                return nullptr;
            
            }
        }

        if(if_stmt->else_stmt) {
            auto scope = std::make_shared<Scope>(current_scope);
            current_scope = scope;

            for(auto& stmt : if_stmt->else_stmt->stmts){
                auto result = evaluate(stmt);
                if(!result) continue;
                if (break_out_types.contains(result->type)) {
                  current_scope = current_scope->parent;
                  return result;
                }
            }

            current_scope = current_scope->parent;
            return nullptr; 
        }
    }

    return nullptr;
};
RunTimeValue SigmaInterpreter::evaluateWhileLoopStatement(std::shared_ptr<WhileLoopStatement> while_loop) {
    while(std::dynamic_pointer_cast<BoolVal>(evaluate(while_loop->expr))->boolean){
        auto scope = std::make_shared<Scope>(current_scope);
        current_scope = scope;
        bool gonna_break = false;
        for(auto& stmt : while_loop->stmts){
            auto result = evaluate(stmt);
            if(!result) continue;
            if(result->type == BreakType){
                gonna_break = true;
                break;
            }
            if(result->type == ContinueType)
                break;
            if(result->type == ReturnType){
                current_scope = current_scope->parent;
                return result;
            }
        }

        current_scope = current_scope->parent;
        if(gonna_break) break;
    }

    return nullptr;
};
RunTimeValue SigmaInterpreter::evaluateForLoopStatement(std::shared_ptr<ForLoopStatement> for_loop) {
    auto scope = std::make_shared<Scope>(current_scope);
    current_scope = scope;
    evaluate(for_loop->first_stmt);

    bool gonna_break = false;
    while(std::dynamic_pointer_cast<BoolVal>(for_loop->expr)->boolean){
        auto sc = std::dynamic_pointer_cast<Scope>(current_scope);
        current_scope = sc;

        for(auto& stmt : for_loop->stmts){
            auto result = evaluate(stmt);
            if(!result) continue;
            if(result->type == BreakType){
                gonna_break = true;
                break;
            }
            else if (result->type == ContinueType)
                break;
            else if (result->type == ReturnType){
                current_scope = current_scope->parent;
                current_scope = current_scope->parent;
                return result;
            }
        }
        if(gonna_break){
            current_scope = current_scope->parent;
            break;
        }

        evaluate(for_loop->last_stmt);
        current_scope = current_scope->parent;
    }

    current_scope = current_scope->parent;

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateProgram(std::shared_ptr<SigmaProgram> program) {
    current_scope = std::make_shared<Scope>(nullptr);

    for(auto& stmt : program->stmts){
        evaluate(stmt);
    }

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateBinaryExpression(std::shared_ptr<BinaryExpression> expr) {
    auto left = evaluate(expr->left);
    auto right = evaluate(expr->right);

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
    
    std::vector<RunTimeValue> args(expr->args.size());

    std::transform(expr->args.begin(), expr->args.end(), args.begin(),
        [this](Expr& expr){ return evaluate(expr); });
    
    if(native_functions.contains(expr->func_name)){    
        return native_functions[expr->func_name](args);
    } else {
        auto func = current_scope->getVal(expr->func_name);
        if(func->type != LambdaType)
            throw std::runtime_error(expr->func_name + " is not a callable");
        auto actual_func = std::dynamic_pointer_cast<LambdaVal>(func);

        auto arg_scope = std::make_shared<Scope>(current_scope);
        current_scope = arg_scope;
        for(int i = 0; i < args.size(); i++){
            current_scope->declareVar(actual_func->params[i], {args[i] , false});
        }
        auto func_scope = std::make_shared<Scope>(current_scope);;
        current_scope = func_scope;

        RunTimeValue return_val;

        for(auto& stmt : actual_func->stmts){
            auto val = evaluate(stmt);
            if(val->type == ReturnType){
                return_val = std::dynamic_pointer_cast<ReturnVal>(val)->val;
                break;
            }
        }

        current_scope = current_scope->parent;
        current_scope = current_scope->parent;
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
    if(op == "+")
        return std::make_shared<StringVal>(left->str + right->str);

    throw std::runtime_error("operator " + op + " isn't valid between operands String, String");
};