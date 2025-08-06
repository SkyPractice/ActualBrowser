#include "Util.h"
#include "../StandardLibrary/ConsoleLib/ConsoleLib.h"
#include "../StandardLibrary/ArrayLib/ArrayLib.h"
#include "../StandardLibrary/DocumentLib/DocumentLib.h"
#include "../StandardLibrary/FilesLib/FilesLib.h"
#include "../StandardLibrary/CryptoLib/CryptoLib.h"
#include "../StandardLibrary/GCLib/GCLib.h"
#include "../StandardLibrary/MathLib/MathLib.h"
#include "../StandardLibrary/PermissionLib/PermissionLib.h"
#include "../StandardLibrary/StdLib.h"
#include "../StandardLibrary/ThreadLib/ThreadLib.h"
#include "../StandardLibrary/TimeLib/TimeLib.h"
#include "../StandardLibrary/MathLib/MathLib.h"
#include "../StandardLibrary/WindowLib/WindowLib.h"
#include "../StandardLibrary/TypeWrappers/ArrayWrapper.h"
#include "../StandardLibrary/TypeWrappers/StringWrapper.h"
#include <algorithm>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <vector>

void Util::SigmaInterpreterHelper::initializeStandardLibraries(std::shared_ptr<Scope>&
    target_scope) {
    target_scope->declareVar("Files", { FilesLib::getStruct() ,true });
    target_scope->declareVar("Console", { ConsoleLib::getStruct() ,true });
    target_scope->declareVar("Time", {TimeLib::getStruct(), true});
    target_scope->declareVar("Array", {ArrayLib::getStruct(), true});
    target_scope->declareVar("Crypto", { CryptoLib::getStruct(), true });
    target_scope->declareVar("Document", { DocumentLib::getStruct(), true });
    target_scope->declareVar("Thread", {ThreadLib::getStruct(), true});
    target_scope->declareVar("GarbageCollector", {GCLib::getStruct(), true});
    target_scope->declareVar("Math", { MathLib::getStruct(), true });
    target_scope->declareVar("Window", {WindowLib::getStruct(), true});
    target_scope->declareVar("Permissions", {PermissionLib::getStruct(), true});
};

LambdaVal* Util::SigmaInterpreterHelper::evaluateLambda(std::shared_ptr<Scope>& target_scope,
    Statement* stmt) {
    auto stm = static_cast<LambdaExpression*>(stmt);
    return RunTimeFactory::makeLambda((stm->params), (stm->stmts),
        std::move(target_scope->flatten()));
};

std::vector<RunTimeVal*> Util::SigmaInterpreterHelper::evaluateExprVector(SigmaInterpreter* self, std::vector<Expression*>& expr_vec) {
    std::vector<RunTimeVal*> results(expr_vec.size());

    std::transform(expr_vec.begin(), expr_vec.end(),
        results.begin(), [self](Expression* expr)
        { return self->evaluate(expr); });
    
    return results;
};

ObjectVal* Util::SigmaInterpreterHelper::evaluateStruct(SigmaInterpreter* self,
    StructExpression* stmt) {
    if(!self->struct_decls.contains(stmt->struct_name))
        throw std::runtime_error("no struct with name " + stmt->struct_name);

    StructDecleration& struct_decleration = self->struct_decls[stmt->struct_name];

    std::vector<VariableDecleration*> vecc = struct_decleration.variable_decls;
    std::unordered_map<std::string, RunTimeVal*> vals;

    LambdaExpression* expr = struct_decleration.constructor;

    if(!expr){

        for(int i = 0; i < stmt->args.size(); i++){
            vals.insert({ vecc[i]->var_name, self->copyIfRecommended(self->evaluate(stmt->args[i])) });
        }

        for(int k = stmt->args.size(); k < vecc.size(); k++){
            vals.insert({ vecc[k]->var_name, self->copyIfRecommended(self->evaluate(vecc[k]->expr))  });
        }

        ObjectVal* object = RunTimeFactory::makeStruct(std::move(vals));

        return object;

    }

    for(auto& var_decl : vecc){
        std::cout << var_decl->var_name << " " << var_decl->expr->type << std::endl;
        vals.insert({ var_decl->var_name, self->copyIfRecommended(self->evaluate(var_decl->expr))  });
    }

    ObjectVal* object = RunTimeFactory::makeStruct(std::move(vals));


    LambdaVal* lambda = Util::SigmaInterpreterHelper::evaluateLambda(self->current_scope, expr);

    std::shared_ptr<Scope> this_keyword_scope = std::make_shared<Scope>(self->current_scope);
    self->current_scope = this_keyword_scope;

    self->current_scope->declareVar("this", { object, true });


    std::vector<RunTimeVal*> evaluated_args(stmt->args.size());
    std::transform(stmt->args.begin(), stmt->args.end(), evaluated_args.begin(),
        [&](Expression* target_expr){
            return self->copyIfRecommended(self->evaluate(target_expr));
        });
    
    if(evaluated_args.size() != lambda->params.size())
        throw std::runtime_error("the argument count of a constructor call didn't match the specified parameter count of the constructor function");
    self->evaluateAnonymousLambdaCall(lambda, {evaluated_args});

    self->current_scope = self->current_scope->parent;

    return object;
    
};

RunTimeVal* Util::SigmaInterpreterHelper::evaluteIdentifier(SigmaInterpreter* self,
    IdentifierExpression* expr) {
    auto sc = self->current_scope->traverse(self->this_str);
    if(sc){
        auto val = sc->variables[self->this_str].value;
        if(val->type == StructType){
            auto vall = static_cast<ObjectVal*>(
                self->current_scope->getVal(self->this_str));
            if(vall->vals.contains(expr->str)){
                return vall->vals[expr->str];
            }
        }
    }
    return self->current_scope->getVal(expr->str);

};

RunTimeVal* Util::SigmaInterpreterHelper::evaluateIfCodeBlock(SigmaInterpreter* self,
    std::vector<Statement*>& stmts) {
    auto scope = std::make_shared<Scope>(self->current_scope);
    self->current_scope = scope;
    for(auto& stmt : stmts){
        auto result = self->evaluate(stmt);
        if(!result) continue;
        if(self->break_out_types.contains(result->type)){
            self->current_scope = self->current_scope->parent;
            return result;
        }
    }
    self->current_scope = self->current_scope->parent;
    self->garbageCollectIfNeeded();
    return nullptr;
};

RunTimeVal* Util::SigmaInterpreterHelper::evaluteLoopCodeBlock(SigmaInterpreter* self,
    std::vector<Statement*>& stmts, bool& request_break) {
    for(auto& stmt : stmts){
        auto result = self->evaluate(stmt);
        if(!result) continue;
        if(result->type == BreakType){
            request_break = true;
            break;
        }
        if(result->type == ContinueType)
            break;
        if(result->type == ReturnType){
            self->current_scope = self->current_scope->parent;
            return result;
        }
    }
    return nullptr;
};

void Util::SigmaInterpreterHelper::verifyCompiledFunctionCallArgs(SigmaInterpreter* self,
    FunctionCallExpression* expr, std::vector<RunTimeVal*>& args, RunTimeVal* func) {
    std::string func_symbol = "<unknown>";
    if(expr->func_expr->type == MemberAccessExpressionType){
        func_symbol = "";
        auto member_access_expr_variant = static_cast<MemberAccessExpression*>(expr->func_expr);
        if(member_access_expr_variant->struct_expr->type == IdentifierExpressionType){
            func_symbol += static_cast<IdentifierExpression*>(member_access_expr_variant->struct_expr)->str;
        }
        for(auto& path_member : member_access_expr_variant->path){
            func_symbol += "." + path_member;
        }
    }
    auto func_val = static_cast<NativeFunctionVal*>(func);
    size_t min_arg_count = func_val->calculateMinimumArgNumber();
    size_t max_arg_count = func_val->calculateMaximumArgNumber();
    if(args.size() > max_arg_count){
        std::ostringstream err_msg_stream;
        err_msg_stream << "function " << func_symbol << " takes a maximum of " <<
            max_arg_count << " args, while " << args.size() << " args were provided";
        throw std::runtime_error(err_msg_stream.str());
    } else if(args.size() < min_arg_count){
        std::ostringstream err_msg_stream;
        err_msg_stream << "function " << func_symbol << " takes a minimum of " <<
            max_arg_count << " args, while " << args.size() << " args were provided";
            
        throw std::runtime_error(err_msg_stream.str());
    }
    for(size_t i = 0; i < args.size(); i++){
        if(func_val->arg_types[i].type == AnyType)
            continue;
        if(func_val->arg_types[i].type != args[i]->type){
            std::ostringstream err_msg_stream;
            err_msg_stream <<   "function call \'" << func_symbol <<
                "\' argument type mismatch in argument number " << i << " an " << 
                "argument of type " << self->type_to_string_table[func_val->arg_types[i].type] << " was excepted but " <<
                "an argument of type " << self->type_to_string_table[args[i]->type] << " was provided";
            throw std::runtime_error(err_msg_stream.str());
        }
    }
};
std::vector<RunTimeVal*> Util::SigmaInterpreterHelper::evaluateExprVectorForCompiledFunctions(
            SigmaInterpreter* self, std::vector<Expression*>& expr_vec
) {
    std::vector<RunTimeVal*> results(expr_vec.size());

    std::transform(expr_vec.begin(), expr_vec.end(),
        results.begin(), [self](Expression* expr)
        { 
            RunTimeVal* val = self->evaluate(expr);
            if(val->type == StructType){
                ObjectVal* obj_val = dynamic_cast<ObjectVal*>(val);
                if(obj_val->vals.contains("is_primitive")){
                    return obj_val->vals["primitive"];
                }
            }
            return val;
        });
    
    return results;  
};

void Util::SigmaInterpreterHelper::cvtToPrimitiveIfWrapper(RunTimeVal** val) {
    RunTimeVal* derefrenced_val = *val;
    if(derefrenced_val->type == StructType) {
        ObjectVal* obj_val = static_cast<ObjectVal*>(derefrenced_val);
        auto itr = obj_val->vals.find("is_primitive");

        if(itr != obj_val->vals.end()){
            RunTimeVal* target_primitive_information = (*itr).second;
            if(target_primitive_information->type == StringType){                
                StringVal* target_string_val = static_cast<StringVal*>(target_primitive_information);

                if(target_string_val->str == "array"){
                    *val = static_cast<ArrayVal*>(obj_val->vals["primitive"]);
                } else if (target_string_val->str == "string"){
                    *val = static_cast<StringVal*>(obj_val->vals["primitive"]);
                }
            }
        }
    }
};

RunTimeVal* Util::SigmaInterpreterHelper::cvtToWrapperIfPossible(RunTimeVal* val) {
    if(val->type == StringType)
        return StringWrapper::genObject(static_cast<StringVal*>(val));
    else if(val->type == ArrayType)
        return ArrayWrapper::genObject(static_cast<ArrayVal*>(val));

    return val;
};