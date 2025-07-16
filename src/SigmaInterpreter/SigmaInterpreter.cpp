#include "SigmaInterpreter.h"
#include "RunTime.h"
#include "SigmaAst.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

SigmaInterpreter::SigmaInterpreter(){

};

RunTimeValue SigmaInterpreter::evaluate(Stmt stmt) {
    switch (stmt->type) {
        case NumericExpressionType:
            return RunTimeFactory::makeNum(std::dynamic_pointer_cast<NumericExpression>(stmt)->num);
        case StringExpressionType:
            return RunTimeFactory::makeString(
                (std::dynamic_pointer_cast<StringExpression>(stmt)->str));
        case BooleanExpressionType:
            return RunTimeFactory::makeBool(std::dynamic_pointer_cast<BoolExpression>(stmt)->val);
        case LambdaExpressionType:{
            auto stm = std::dynamic_pointer_cast<LambdaExpression>(stmt);
            return RunTimeFactory::makeLambda((stm->params), (stm->stmts));
        }
        case ArrayExpressionType:{
            auto stm = std::dynamic_pointer_cast<ArrayExpression>(stmt);
            std::vector<RunTimeValue> vals(stm->exprs.size());
            std::transform(stm->exprs.begin(), stm->exprs.end(), vals.begin(),
                [this](Expr expr){ return evaluate(expr); });
            return RunTimeFactory::makeArray((vals));
        }
        case StructExpressionType:{
            auto stm = std::dynamic_pointer_cast<StructExpression>(stmt);
            if(!struct_decls.contains(stm->struct_name))
                throw std::runtime_error("no struct with name " + stm->struct_name);
            std::vector<std::shared_ptr<VariableDecleration>> vecc = struct_decls[stm->struct_name];
            std::unordered_map<std::string, RunTimeValue> vals;

            for(int i = 0; i < stm->args.size(); i++){
                vals.insert({ vecc[i]->var_name, evaluate(stm->args[i]) });
            }

            for(int k = stm->args.size(); k < vecc.size(); k++){
                vals.insert({ vecc[k]->var_name, evaluate(vecc[k]->expr)  });
            }

            return RunTimeFactory::makeStruct((vals));
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
            return RunTimeFactory::makeContinue();
        case BreakStatementType:
            return RunTimeFactory::makeBreak();
        case ReturnStatementType:
            return RunTimeFactory::makeReturn(
                evaluate(std::dynamic_pointer_cast<ReturnStatement>(stmt)->expr));
        case IndexAccessExpressionType:
            return evaluateIndexAccessExpression(std::dynamic_pointer_cast<IndexAccessExpression>(stmt));
        case IndexReInitStatementType:
            return evaluateIndexReInitStatement(std::dynamic_pointer_cast<IndexReInitStatement>(stmt));
        case StructDeclerationType:
            return evaluateStructDeclStatement(std::dynamic_pointer_cast<StructDeclerationStatement>(stmt));
        case MemberAccessExpressionType:
            return evaluateMemberAccessExpression(std::dynamic_pointer_cast<MemberAccessExpression>(stmt));
        case MemberReInitExpressionType:
            return evaluateMemberReInitStatement(std::dynamic_pointer_cast<MemberReInitExpression>(stmt));
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
    while(std::dynamic_pointer_cast<BoolVal>(evaluate(for_loop->expr))->boolean){
        auto sc = std::make_shared<Scope>(current_scope);
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
    struct_decls.clear();
    
    std::unordered_map<std::string, RunTimeValue> console_vals;
    console_vals.insert({"println", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::println)});
    console_vals.insert({"print", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::print)});
    console_vals.insert({"toString", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::toString)});
    
    std::unordered_map<std::string, RunTimeValue> io_vals;
    io_vals.insert({"readFileSync", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::readFileSync)});
    io_vals.insert({"writeFileSync", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::writeFileSync)});
    
    std::unordered_map<std::string, RunTimeValue> str_vals;
    str_vals.insert({"valueOf", RunTimeFactory::makeNativeFunction(&SigmaInterpreter::toString)});

    current_scope->declareVar("Files", { RunTimeFactory::makeStruct((io_vals)) ,true });
    current_scope->declareVar("Console", { RunTimeFactory::makeStruct((console_vals)) ,true });
    current_scope->declareVar("String", { RunTimeFactory::makeStruct((str_vals)), true });
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
    
    // if(expr->func_expr->type == IdentifierExpressionType){
    //     std::string name = std::dynamic_pointer_cast<IdentifierExpression>(expr->func_expr)->str;
    //     std::cout << "Function name: " << name << std::endl;
    //     if(native_functions.contains(name)){    
    //         return native_functions[name](args);
    //     }
    // }
    auto func = evaluate(expr->func_expr);
    if(func->type == NativeFunctionType){
        auto ac_func = std::dynamic_pointer_cast<NativeFunctionVal>(func);
        return ac_func->func(args);
    }
    if (func->type != LambdaType)
      throw std::runtime_error("<obj> is not a callable");
    auto actual_func = std::dynamic_pointer_cast<LambdaVal>(func);

    auto arg_scope = std::make_shared<Scope>(current_scope);
    current_scope = arg_scope;
    for (int i = 0; i < args.size(); i++) {
      current_scope->declareVar(actual_func->params[i], {args[i], false});
    }
    auto func_scope = std::make_shared<Scope>(current_scope);
    ;
    current_scope = func_scope;

    RunTimeValue return_val;

    for (auto &stmt : actual_func->stmts) {
      auto val = evaluate(stmt);
      if (!val)
        continue;
      if (val->type == ReturnType) {
        return_val = std::dynamic_pointer_cast<ReturnVal>(val)->val;
        break;
      }
    }

    current_scope = current_scope->parent;
    current_scope = current_scope->parent;

    return return_val;
};

RunTimeValue SigmaInterpreter::
    evaluateIndexAccessExpression(std::shared_ptr<IndexAccessExpression> expr) {
    auto val = evaluate(expr->array_expr);
    
    for(auto& num : expr->path){
        
        auto numb = evaluate(num);

        if(numb->type != NumType) throw std::runtime_error("operator [] excepts a number");

        auto real_num = std::dynamic_pointer_cast<NumVal>(numb);
        if(val->type == StringType){
            auto real_val = std::dynamic_pointer_cast<StringVal>(val);    
            if(real_num->num >= real_val->str.size()) throw std::runtime_error("out of bounds array index");
            val = RunTimeFactory::makeString(std::string(1, real_val->str[static_cast<int>(real_num->num)]));
            return val;
        }

        if(val->type != ArrayType) throw std::runtime_error("operator [] must be used on an array");
        auto real_val = std::dynamic_pointer_cast<ArrayVal>(val);

        if(real_num->num >= real_val->vals.size()) throw std::runtime_error("out of bounds array index");
        val = real_val->vals[static_cast<int>(real_num->num)];
    }


    return val;
};

RunTimeValue SigmaInterpreter::
    evaluateMemberAccessExpression(std::shared_ptr<MemberAccessExpression> expr) {
    auto val = evaluate(expr->struct_expr);
    
    for(auto& str : expr->path){
        if(val->type != StructType) throw std::runtime_error("operator . must be used on an object the current type is: " + 
            std::to_string(val->type));
        auto real_val = std::dynamic_pointer_cast<StructVal>(val);

        if(!real_val->vals.contains(str)) throw std::runtime_error("member " + str + " not found in an object");
        
        val = real_val->vals[str];

    }


    return val;
};

RunTimeValue SigmaInterpreter::evaluateIndexReInitStatement(std::shared_ptr<IndexReInitStatement> stmt) {
    auto val = evaluate(stmt->array_expr);
    auto latest_num = 
        std::dynamic_pointer_cast<NumVal>(evaluate(stmt->path.back()));
    stmt->path.pop_back();

    for(auto& num : stmt->path){
        val = std::dynamic_pointer_cast<ArrayVal>(val)->vals[
            static_cast<int>(std::dynamic_pointer_cast<NumVal>(num)->num)
        ];
    }

    if(val->type == StringType){
        auto latest_val = std::dynamic_pointer_cast<StringVal>(val);
        latest_val->str[static_cast<int>(latest_num->num)] = 
            std::dynamic_pointer_cast<StringVal>(evaluate(stmt->val))->str[0]; 
        return nullptr;
    }
    auto latest_val = std::dynamic_pointer_cast<ArrayVal>(val);
    latest_val->vals[static_cast<int>(latest_num->num)] = evaluate(stmt->val);
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateNumericBinaryExpression(std::shared_ptr<NumVal> left,
    std::shared_ptr<NumVal> right, std::string operat) {
    if(operat == "+")
        return RunTimeFactory::makeNum(left->num + right->num);
    if(operat == "-")
        return RunTimeFactory::makeNum(left->num - right->num);
    if(operat == "*")
        return RunTimeFactory::makeNum(left->num * right->num);
    if(operat == "/")
        return RunTimeFactory::makeNum(left->num / right->num);
    if(operat == "%")
        return RunTimeFactory::makeNum((int)left->num % (int)right->num);
    if(operat == "&")
        return RunTimeFactory::makeNum((int)left->num & (int)right->num);
    if(operat == "|")
        return RunTimeFactory::makeNum((int)left->num | (int)right->num);
    if(operat == ">>")
        return RunTimeFactory::makeNum((int)left->num >> (int)right->num);
    if(operat == "<<")
        return RunTimeFactory::makeNum((int)left->num << (int)right->num);
    if(operat == "==")
        return RunTimeFactory::makeBool(left->num == right->num);
    if(operat == ">")
        return RunTimeFactory::makeBool(left->num > right->num);
    if(operat == "<")
        return RunTimeFactory::makeBool(left->num < right->num);
    if(operat == ">=")
        return RunTimeFactory::makeBool(left->num >= right->num);
    if(operat == "<=")
        return RunTimeFactory::makeBool(left->num <= right->num);
    if(operat == "!=")
        return RunTimeFactory::makeBool(left->num != right->num);

    throw std::runtime_error("operator " + operat + " isn't valid between operands Number, Number");
};
RunTimeValue SigmaInterpreter::evaluateBooleanBinaryExpression(std::shared_ptr<BoolVal> left,
    std::shared_ptr<BoolVal> right, std::string op) {
    if(op == "==")
        return RunTimeFactory::makeBool(left->boolean == right->boolean);
    if(op == "!=")
        return RunTimeFactory::makeBool(left->boolean != right->boolean);
    if(op == ">")
        return RunTimeFactory::makeBool(left->boolean > right->boolean);
    if(op == "<")
        return RunTimeFactory::makeBool(left->boolean < right->boolean);
    if(op == ">=")
        return RunTimeFactory::makeBool(left->boolean >= right->boolean);
    if(op == "<=")
        return RunTimeFactory::makeBool(left->boolean <= right->boolean);
    if(op == "|")
        return RunTimeFactory::makeNum(left->boolean | right->boolean);
    if(op == "&")
        return RunTimeFactory::makeNum(left->boolean & right->boolean);
    if(op == "&&")
        return RunTimeFactory::makeBool(left->boolean && right->boolean);
    if(op == "||")
        return RunTimeFactory::makeBool(left->boolean || right->boolean);
    if(op == ">>")
        return RunTimeFactory::makeNum(left->boolean >> right->boolean);
    if(op == "<<")
        return RunTimeFactory::makeNum(left->boolean << right->boolean);

    throw std::runtime_error("operator " + op + " isn't valid between operands Boolean, Boolean");
};
// deprecated
RunTimeValue SigmaInterpreter::evaluateStringBinaryExpression(std::shared_ptr<StringVal> left,
    std::shared_ptr<StringVal> right, std::string op) {
    if(op == "==")
        return RunTimeFactory::makeBool(left->str == right->str);
    if(op == "!=")
        return RunTimeFactory::makeBool(left->str != right->str);
    if(op == ">")
        return RunTimeFactory::makeBool(left->str > right->str);
    if(op == "<")
        return RunTimeFactory::makeBool(left->str < right->str);
    if(op == ">=")
        return RunTimeFactory::makeBool(left->str >= right->str);
    if(op == "<=")
        return RunTimeFactory::makeBool(left->str <= right->str);
    if(op == "+")
        return RunTimeFactory::makeString(left->str + right->str);

    throw std::runtime_error("operator " + op + " isn't valid between operands String, String");
};

RunTimeValue SigmaInterpreter::
    evaluateStructDeclStatement(std::shared_ptr<StructDeclerationStatement> stmt) {
    struct_decls.insert({stmt->struct_name, stmt->props});

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateMemberReInitStatement(
    std::shared_ptr<MemberReInitExpression> expr) {
    auto val = evaluate(expr->struct_expr);
    auto latest_str = expr->path.back();
    expr->path.pop_back();

    for(auto& str : expr->path){
        val = std::dynamic_pointer_cast<StructVal>(val)->vals[
            str
        ];
    }

    auto latest_val = std::dynamic_pointer_cast<StructVal>(val);
    latest_val->vals[latest_str] = evaluate(expr->val);
    return nullptr;
};