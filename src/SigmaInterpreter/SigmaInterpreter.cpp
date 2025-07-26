#include "SigmaInterpreter.h"
#include "RunTime.h"
#include "SigmaAst.h"
#include <algorithm>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "../Interpreter/Interpreter.h"
#include "../Interpreter/Parser.h"
#include "StandardLibrary/ConsoleLib/ConsoleLib.h"
#include "StandardLibrary/ArrayLib/ArrayLib.h"
#include "StandardLibrary/FilesLib/FilesLib.h"
#include "StandardLibrary/CryptoLib/CryptoLib.h"
#include "StandardLibrary/StdLib.h"
#include "StandardLibrary/ThreadLib/ThreadLib.h"


SigmaInterpreter::SigmaInterpreter(){
};

std::unordered_set<RunTimeValType> SigmaInterpreter::non_copyable_types = {
        StructType, LambdaType, StringType, ArrayType, BinaryType, NativeFunctionType, HtmlType
};

void SigmaInterpreter::initialize(){
    current_scope = std::make_shared<Scope>(nullptr);
    struct_decls.clear();
    
    std::unordered_map<std::string, RunTimeValue> str_vals;
    str_vals.insert({"valueOf", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::toString, this, std::placeholders::_1))});

    std::unordered_map<std::string, RunTimeValue> obj_vals;

    obj_vals.insert({"ref", RunTimeFactory::makeNativeFunction(
        [this](std::vector<RunTimeVal*>& vals) { 
        return RunTimeFactory::makeRefrence(&vals[0]);
        }
    )    
    });
    obj_vals.insert({"valByRef", RunTimeFactory::makeNativeFunction(
        [this](std::vector<RunTimeVal*>& vals) { 
            return *static_cast<RefrenceVal*>(vals[0])->val;
        }
    )    
    });
    obj_vals.insert({"clone", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::clone, this, std::placeholders::_1))});

    std::unordered_map<std::string, RunTimeValue> tim_vals;
    tim_vals.insert({"getCurrentTimeMillis", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::getCurrentTimeMillis, this, std::placeholders::_1))});
    std::unordered_map<std::string, RunTimeValue> dom_vals;
    dom_vals.insert({"getElementById", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::getElementById, this, std::placeholders::_1))});
    dom_vals.insert({"getElementsByClassName", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::getElementsByClassName, this, std::placeholders::_1))});
    dom_vals.insert({"setInnerHtml", RunTimeFactory::makeNativeFunction(std::bind(&SigmaInterpreter::setElementInnerHtml, this, std::placeholders::_1))});

    current_scope->declareVar("Files", { FilesLib::getStruct() ,true });
    current_scope->declareVar("Console", { ConsoleLib::getStruct() ,true });
    current_scope->declareVar("String", { RunTimeFactory::makeStruct((str_vals)), true });
    current_scope->declareVar("Object", { RunTimeFactory::makeStruct((obj_vals)), true });
    current_scope->declareVar("Time", {RunTimeFactory::makeStruct(tim_vals), true});
    current_scope->declareVar("Array", {ArrayLib::getStruct(), true});
    current_scope->declareVar("Crypto", { CryptoLib::getStruct(), true });
    current_scope->declareVar("Document", { RunTimeFactory::makeStruct(dom_vals), true });
    current_scope->declareVar("Thread", {ThreadLib::getStruct(), true});
}

RunTimeValue SigmaInterpreter::evaluate(Stmt stmt) {
    switch (stmt->type) {
        case NumericExpressionType:
            return RunTimeFactory::makeNum(static_cast<NumericExpression*>(stmt)->num);
        case StringExpressionType:
            return RunTimeFactory::makeString(
                (static_cast<StringExpression*>(stmt)->str));
        case BooleanExpressionType:
            return RunTimeFactory::makeBool(static_cast<BoolExpression*>(stmt)->val);
        case LambdaExpressionType:{
            auto stm = static_cast<LambdaExpression*>(stmt);
            return RunTimeFactory::makeLambda((stm->params), (stm->stmts),
                std::move(current_scope->flatten()));
        }
        case ArrayExpressionType:{
            auto stm = static_cast<ArrayExpression*>(stmt);
            std::vector<RunTimeVal*> vals(stm->exprs.size());
            std::transform(stm->exprs.begin(), stm->exprs.end(), vals.begin(),
                [this](Expr expr){ return evaluate(expr); });
            return RunTimeFactory::makeArray(std::move(vals));
        }
        case StructExpressionType:{
            auto stm = static_cast<StructExpression*>(stmt);
            if(!struct_decls.contains(stm->struct_name))
                throw std::runtime_error("no struct with name " + stm->struct_name);
            std::vector<VariableDecleration*> vecc = struct_decls[stm->struct_name];
            std::unordered_map<std::string, RunTimeVal*> vals;

            for(int i = 0; i < stm->args.size(); i++){
                vals.insert({ vecc[i]->var_name, evaluate(stm->args[i]) });
            }

            for(int k = stm->args.size(); k < vecc.size(); k++){
                vals.insert({ vecc[k]->var_name, evaluate(vecc[k]->expr)  });
            }

            return RunTimeFactory::makeStruct(std::move(vals));
        }
        case BinaryExpressionType:
            return evaluateBinaryExpression(static_cast<BinaryExpression*>(stmt));
        case ProgramType:
            return evaluateProgram(static_cast<SigmaProgram*>(stmt));    
        case VariableDeclerationType:
            return evaluateVariableDeclStatement(static_cast<VariableDecleration*>(stmt));
        case VariableReInitializationType:
            return evaluateVariableReInitStatement(static_cast<VariableReInit*>(stmt));
        case FunctionCallExpressionType:
            return evaluateFunctionCallExpression(static_cast<FunctionCallExpression*>(stmt));
        case IdentifierExpressionType:{
            std::string target = static_cast<IdentifierExpression*>(stmt)->str;
            auto sc = current_scope->traverse(this_str);
            if(sc){
                auto val = sc->variables[this_str].value;
                if(val->type == StructType){
                    auto vall = static_cast<StructVal*>(
                        current_scope->getVal(this_str));
                    if(vall->vals.contains(target)){
                        return vall->vals[target];
                    }
                }
            }
            return current_scope->getVal(static_cast<IdentifierExpression*>(stmt)->str);
        }
        case IfStatementType:
            return evaluateIfStatement(static_cast<IfStatement*>(stmt));
        case WhileStatementType:
            return evaluateWhileLoopStatement(static_cast<WhileLoopStatement*>(stmt));
        case ForStatementType:
            return evaluateForLoopStatement(static_cast<ForLoopStatement*>(stmt));
        case ContinueStatementType:
            return RunTimeFactory::makeContinue();
        case BreakStatementType:
            return RunTimeFactory::makeBreak();
        case ReturnStatementType:
            return RunTimeFactory::makeReturn(
                evaluate(static_cast<ReturnStatement*>(stmt)->expr));
        case IndexAccessExpressionType:
            return evaluateIndexAccessExpression(static_cast<IndexAccessExpression*>(stmt));
        case IndexReInitStatementType:
            return evaluateIndexReInitStatement(static_cast<IndexReInitStatement*>(stmt));
        case StructDeclerationType:
            return evaluateStructDeclStatement(static_cast<StructDeclerationStatement*>(stmt));
        case MemberAccessExpressionType:
            return evaluateMemberAccessExpression(static_cast<MemberAccessExpression*>(stmt));
        case MemberReInitExpressionType:
            return evaluateMemberReInitStatement(static_cast<MemberReInitExpression*>(stmt));
        case IncrementExpressionType:
            return evaluateIncrementExpression(static_cast<IncrementExpression*>(stmt));
        case NegativeExpressionType:
            return evaluateNegativeExpression(static_cast<NegativeExpression*>(stmt));
        default: throw std::runtime_error("Not Implemented " + std::to_string(stmt->type));
    }
};

RunTimeValue SigmaInterpreter::evaluateIfStatement(IfStatement* if_stmt) {
    auto expr = evaluate(if_stmt->expr);
    if(expr->type != BoolType)
        throw std::runtime_error("if statement expression must result in a boolean value");
    if(static_cast<BoolVal*>(expr)->boolean){
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
        
            if(static_cast<BoolVal*>(else_if_expr)->boolean){
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
RunTimeValue SigmaInterpreter::evaluateWhileLoopStatement(WhileLoopStatement* while_loop) {
    auto scope = std::make_shared<Scope>(current_scope);
    current_scope = scope;
    while(static_cast<BoolVal*>(evaluate(while_loop->expr))->boolean){

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

        if(gonna_break) break;
        current_scope->variables.clear();
    }
    current_scope = current_scope->parent;
    return nullptr;
};
RunTimeValue SigmaInterpreter::evaluateForLoopStatement(ForLoopStatement* for_loop) {
    auto scope = std::make_shared<Scope>(current_scope);
    current_scope = scope;
    if(for_loop->first_stmt)
        evaluate(for_loop->first_stmt);

    auto sc = std::make_shared<Scope>(current_scope);
    current_scope = sc;
    bool gonna_break = false;
    while(static_cast<BoolVal*>(evaluate(for_loop->expr))->boolean){


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

        if(for_loop->last_stmt)
            evaluate(for_loop->last_stmt);

        if(!current_scope->variables.empty())
            current_scope->variables.clear();    
    }
    current_scope = current_scope->parent;
    current_scope = current_scope->parent;

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateProgram(SigmaProgram* program) {
    initialize();
    auto v = static_cast<ForLoopStatement*>(program->stmts[0]);
    for(auto& stmt : program->stmts){
        evaluate(stmt);
    }

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateBinaryExpression(BinaryExpression* expr) {
    auto left = evaluate(expr->left);
    auto right = evaluate(expr->right);

    if(left->type == NumType && right->type == NumType){
        auto l = static_cast<NumVal*>(left);
        auto r = static_cast<NumVal*>(right);
        return evaluateNumericBinaryExpression(l, r, expr->op);
    }
    if(left->type == StringType && right->type == StringType){
        auto l = static_cast<StringVal*>(left);
        auto r = static_cast<StringVal*>(right);

        return evaluateStringBinaryExpression(l, r, expr->op);
    }
    if(left->type == BoolType && right->type == BoolType){
        auto l = static_cast<BoolVal*>(left);
        auto r = static_cast<BoolVal*>(right);

        return evaluateBooleanBinaryExpression(l, r, expr->op);
    }

    throw std::runtime_error("Binary Operators Not Implemented For Operands " + 
        std::to_string((int)left->type) + "," + std::to_string((int)right->type));
};

RunTimeValue SigmaInterpreter::evaluateVariableDeclStatement(VariableDecleration* decl) {
    auto val = evaluate(decl->expr);
    if(!shouldICopy(val)){
        current_scope->declareVar(decl->var_name, { val,
         decl->is_const });
        return nullptr;
    }
    current_scope->declareVar(decl->var_name, { val->clone(),
         decl->is_const });
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateVariableReInitStatement(VariableReInit* decl) {
    auto scc = current_scope->traverse(this_str);
    if(scc){
        auto this_val = scc->variables[this_str].value;
        if(this_val->type == StructType){
            auto vall = static_cast<StructVal*>(
                this_val);
            if(vall->vals.contains(decl->var_name)){
                auto v = evaluate(decl->expr);
                auto stru = static_cast<StructVal*>(this_val);
                if(stru->vals[decl->var_name]->type == RefrenceType){
                    auto actual_ref = static_cast<RefrenceVal*>(stru->vals[decl->var_name]);
                    *actual_ref->val = v;
                    return nullptr;
                }
                if(shouldICopy(v))
                    stru->vals[decl->var_name]->setValue(v);
                else {
                    stru->vals[decl->var_name] = v;
                }
                return nullptr;
            }
        }
    }
    auto ac_v = evaluate(decl->expr);
    auto v = current_scope->getVal(decl->var_name);
    if(v->type == RefrenceType){
        auto actual_ref = 
            static_cast<RefrenceVal*>(v);
        *actual_ref->val = (ac_v);
        return nullptr;
    }
    if(shouldICopy(v)){
        v->setValue(ac_v);
        return nullptr;
    }
    current_scope->reInitVar(decl->var_name, (ac_v));
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateFunctionCallExpression(FunctionCallExpression* expr) {
    
    std::vector<RunTimeVal*> args(expr->args.size());
    auto func = evaluate(expr->func_expr);

    if(func->type == LambdaType)
    std::transform(expr->args.begin(), expr->args.end(), args.begin(),
        [this](Expr expr)-> RunTimeVal* { 
            auto va = evaluate(expr);
            return copyIfRecommended(va);
        });
    else if(func->type == NativeFunctionType){
        StdLib::current_calling_scope = current_scope;
        std::transform(expr->args.begin(), expr->args.end(), args.begin(),
        [this](Expr expr){ 
            auto va = evaluate(expr);
            return va;
        });
    }
    // if(expr->func_expr->type == IdentifierExpressionType){
    //     std::string name = static_cast<IdentifierExpression*>(expr->func_expr)->str;
    //     std::cout << "Function name: " << name << std::endl;
    //     if(native_functions.contains(name)){    
    //         return native_functions[name](args);
    //     }
    // }
    
    if(func->type == NativeFunctionType){
        auto sc = std::make_shared<Scope>(current_scope);
        auto last_sc = current_scope;
        current_scope = sc;
        if(expr->func_expr->type == MemberAccessExpressionType){
            auto mem_expr = static_cast<MemberAccessExpression*>(expr->func_expr)->clone_expr();
            mem_expr->path.pop_back();
            auto mem_val = evaluate(mem_expr);
            current_scope->declareVar(this_str, { mem_val, true });
        }
        auto ac_func = static_cast<NativeFunctionVal*>(func);
        auto ac_vv = ac_func->func(args);
        current_scope = last_sc;
        StdLib::current_calling_scope = nullptr;
        return ac_vv;
    }
    if (func->type != LambdaType)
      throw std::runtime_error(std::format("{} is not a callable", (int)func->type));
    auto actual_func = static_cast<LambdaVal*>(func);
    
    auto last_scope = current_scope;

    auto arg_scope = std::make_shared<Scope>(current_scope);
    current_scope = arg_scope;
    for (int i = 0; i < args.size(); i++) {
      current_scope->declareVar(actual_func->params[i], {args[i], false});
    }
    // for(auto& [var_name, var_val] : actual_func->captured){
    //     if(!current_scope->variables.contains(var_name))
    //         current_scope->declareVar(var_name, {var_val, true});
    // }
    if(expr->func_expr->type == MemberAccessExpressionType){
        auto mem_expr = static_cast<MemberAccessExpression*>(expr->func_expr);
        mem_expr->path.pop_back();
        current_scope = last_scope;
        auto mem_val = evaluate(mem_expr);
        current_scope = arg_scope;
        current_scope->declareVar(this_str, { mem_val, true });

    }

    auto func_scope = std::make_shared<Scope>(current_scope);
    
    current_scope = func_scope;

    RunTimeValue return_val;

    for (auto &stmt : actual_func->stmts) {
      auto val = evaluate(stmt);
      if (!val)
        continue;
      if (val->type == ReturnType) {
        return_val = static_cast<ReturnVal*>(val)->val;
        break;
      }
    }

    current_scope = current_scope->parent;
    current_scope = last_scope;

    return return_val;
};

RunTimeValue SigmaInterpreter::
    evaluateIndexAccessExpression(IndexAccessExpression* expr) {
    auto val = evaluate(expr->array_expr);
    
    for(auto& num : expr->path){
        
        auto numb = evaluate(num);

        if(numb->type != NumType) throw std::runtime_error("operator [] excepts a number");

        auto real_num = static_cast<NumVal*>(numb);
        if(val->type == StringType){
            auto real_val = static_cast<StringVal*>(val);    
            if(real_num->num >= real_val->str.size()) throw std::runtime_error("out of bounds array index");
            val = RunTimeFactory::makeString(std::string(1, real_val->str[static_cast<int>(real_num->num)]));
            return val;
        }

        if(val->type != ArrayType) throw std::runtime_error("operator [] must be used on an array");
        auto real_val = static_cast<ArrayVal*>(val);

        if(real_num->num >= real_val->vals.size()) throw std::runtime_error("out of bounds array index");
        val = real_val->vals[static_cast<int>(real_num->num)];
    }


    return val;
};

RunTimeValue SigmaInterpreter::
    evaluateMemberAccessExpression(MemberAccessExpression* expr) {
    auto val = evaluate(expr->struct_expr);
    
    for(auto& str : expr->path){
        if(val->type != StructType) throw std::runtime_error("operator . must be used on an object the current type is: " + 
            std::to_string(val->type));
        auto real_val = static_cast<StructVal*>(val);

        if(!real_val->vals.contains(str)) throw std::runtime_error("member " + str + " not found in an object");
        
        val = real_val->vals[str];

    }


    return val;
};

RunTimeValue SigmaInterpreter::evaluateIndexReInitStatement(IndexReInitStatement* stmt) {
    auto val = evaluate(stmt->array_expr);
    auto latest_num = 
        static_cast<NumVal*>(evaluate(stmt->path.back()));
    stmt->path.pop_back();

    for(auto& num : stmt->path){
        val = static_cast<ArrayVal*>(val)->vals[
            static_cast<int>(static_cast<NumVal*>(evaluate(num))->num)
        ];
    }

    if(val->type == StringType){
        auto latest_val = static_cast<StringVal*>(val);
        latest_val->str[static_cast<int>(latest_num->num)] = 
            static_cast<StringVal*>(evaluate(stmt->val))->str[0]; 
        return nullptr;
    }
    auto latest_val = static_cast<ArrayVal*>(val);
    if(latest_val->vals[static_cast<int>(latest_num->num)]->type == RefrenceType){
        auto v = 
            static_cast<RefrenceVal*>(latest_val->vals[static_cast<int>(latest_num->num)]);
        *v->val = evaluate(stmt->val);
        return nullptr;
    }
    auto actual_val = evaluate(stmt->val);

    if(shouldICopy(actual_val))
        latest_val->vals[static_cast<int>(latest_num->num)]->setValue(actual_val);
    else {
        latest_val->vals[static_cast<int>(latest_num->num)] = actual_val;
    }
    
    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateNumericBinaryExpression(NumVal* left,
    NumVal* right, std::string operat) {
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
RunTimeValue SigmaInterpreter::evaluateBooleanBinaryExpression(BoolVal* left,
    BoolVal* right, std::string op) {
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
RunTimeValue SigmaInterpreter::evaluateStringBinaryExpression(StringVal* left,
    StringVal* right, std::string op) {
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
    evaluateStructDeclStatement(StructDeclerationStatement* stmt) {
    struct_decls.insert({stmt->struct_name, stmt->props});

    return nullptr;
};

RunTimeValue SigmaInterpreter::evaluateMemberReInitStatement(
    MemberReInitExpression* expr) {
    auto val = evaluate(expr->struct_expr);
    auto path_copy = expr->path;
    auto latest_str = path_copy.back();
    path_copy.pop_back();

    for(auto& str : path_copy){
        val = static_cast<StructVal*>(val)->vals[
            str
        ];
    }

    auto latest_val = static_cast<StructVal*>(val);
    if(latest_val->vals[latest_str]->type == RefrenceType){
        auto v = static_cast<RefrenceVal*>(latest_val->vals[latest_str]);
        *v->val = evaluate(expr);
        return nullptr;
    }
    auto actual_v = evaluate(expr->val);
    if(shouldICopy(actual_v))
        latest_val->vals[latest_str]->setValue(actual_v);
    else {
        latest_val->vals[latest_str] = actual_v;
    }
    return nullptr;
};


RunTimeValue SigmaInterpreter::evaluateIncrementExpression(IncrementExpression* expr) {
    auto current_val = evaluate(expr->expr);

    if(current_val->type != NumType) throw std::runtime_error("can't increment a non-number");
    double actual_val = static_cast<NumVal*>(current_val)->num;
    if(expr->expr->type == IdentifierExpressionType){
        if(!expr->cached_variable_reinit){
            auto iden_expr = static_cast<IdentifierExpression*>(expr->expr);
            expr->cached_variable_reinit = SigmaParser::makeAst<VariableReInit>((iden_expr->str), 
                SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount));
        }
        else {
            static_cast<NumericExpression*>(
                expr->cached_variable_reinit->expr)->num = actual_val + expr->amount;
        }
        evaluateVariableReInitStatement(expr->cached_variable_reinit);
    } else if (expr->expr->type == MemberAccessExpressionType){
        if(!expr->cached_member_reinit){
            auto ac_expr = static_cast<MemberAccessExpression*>(expr->expr);
            expr->cached_member_reinit = SigmaParser::makeAst<MemberReInitExpression>(
                ac_expr->struct_expr, ac_expr->path, SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount)
            );
        } else { static_cast<NumericExpression*>(expr->cached_member_reinit->val)->num = actual_val + expr->amount; }
        evaluateMemberReInitStatement(expr->cached_member_reinit);
    } else if (expr->expr->type == ArrayExpressionType){
        if(!expr->cached_index_reinit){
            auto ac_expr = static_cast<IndexAccessExpression*>(expr->expr);
            expr->cached_index_reinit = SigmaParser::makeAst<IndexReInitStatement>(
                ac_expr->array_expr, ac_expr->path, SigmaParser::makeAst<NumericExpression>(actual_val + expr->amount)
            );
        } else { static_cast<NumericExpression*>(expr->cached_index_reinit->val)->num = actual_val + expr->amount; }
        evaluateIndexReInitStatement(expr->cached_index_reinit);
    }
    return RunTimeFactory::makeNum(actual_val + expr->amount);
};
RunTimeValue SigmaInterpreter::evaluateDecrementExpression(DecrementExpression* expr) {
    return {};
};
RunTimeValue SigmaInterpreter::evaluateNegativeExpression(NegativeExpression* expr) {
    auto val = evaluate(expr->expr);
    if(val->type != NumType)
        throw std::runtime_error("can't make a non-number value negative");

    double num = static_cast<NumVal*>(val)->num;

    return RunTimeFactory::makeNum(-num);
};
RunTimeValue SigmaInterpreter::evaluateHtmlStr(StringExpression* expr){
    Lexer lexer;
    Parser parser;
    auto tokens = lexer.tokenize(expr->str);
    Program ast_representation = parser.produceAst(tokens);

    std::vector<RunTimeVal*> html_elms(ast_representation.html_tags.size());
    std::vector<RunTimeVal*> style_srcs(ast_representation.style_srcs.size());
    std::vector<RunTimeVal*> script_srcs(ast_representation.script_srcs.size());

    std::transform(ast_representation.html_tags.begin(), ast_representation.html_tags.end(),
        html_elms.begin(), [](std::shared_ptr<HTMLTag> elm)-> HtmlElementVal* { 
            return RunTimeFactory::makeHtmlElement(elm.get());
        });
    std::transform(ast_representation.script_srcs.begin(), ast_representation.script_srcs.end(),
        script_srcs.begin(), [](std::string str){ 
            return RunTimeFactory::makeString(str);
        });
    std::transform(ast_representation.style_srcs.begin(), ast_representation.style_srcs.end(),
        style_srcs.begin(), [](std::string str){ 
            return RunTimeFactory::makeString(str);
        });
    
    std::unordered_map<std::string, RunTimeVal*> struct_map = {
        {"elements", RunTimeFactory::makeArray(std::move(html_elms))},
        {"style_srcs", RunTimeFactory::makeArray(std::move(style_srcs))},
        {"script_srcs", RunTimeFactory::makeArray(std::move(script_srcs))}
    };

    return RunTimeFactory::makeStruct(std::move(struct_map));
};