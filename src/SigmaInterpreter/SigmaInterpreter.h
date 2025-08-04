#pragma once
#include "RunTime.h"
#include "SigmaAst.h"
#include <functional>
#include <memory>
#include "Util/Permissions/Permissions.h"
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Variable {
    RunTimeVal* value;
    bool is_const;
};

struct DOMAccessor;

class Scope : public std::enable_shared_from_this<Scope>{
public: 
    std::shared_ptr<Scope> parent;
    std::unordered_map<std::string, Variable> variables;
    std::unordered_map<std::string, Scope*> cache;
    PermissionContainer perms;

    Scope(std::shared_ptr<Scope> p): parent(p) {};

    std::shared_ptr<Scope> traverse(const std::string& var_name){
        if(variables.contains(var_name)) return shared_from_this();
        else if (parent) return parent->traverse(var_name);
        return nullptr;
    };

    RunTimeVal* getVal(std::string& var_name){
        auto itr = cache.find(var_name);
        bool result = (itr != cache.end());
        if(!result){
            auto scope = traverse(var_name);
            if(!scope){ throw std::runtime_error("variable " + var_name + " not found"); };
            cache.insert({ var_name, scope.get() });
            return scope->variables[var_name].value;
        }
        return itr->second->variables.find(var_name)->second.value;
    };

    // shadowing is allowed
    void declareVar(std::string name, Variable val){
        val.value->is_l_val = true;
        variables[name] = val;
    };

    void reInitVar(std::string& name, RunTimeVal* val){
        val->is_l_val = true;
        auto itr = cache.find(name);
        bool result = (itr != cache.end());
        if(!result){
            auto scope = traverse(name);
            if(!scope) { throw std::runtime_error("identifier " + name + " not found"); };
            auto& another_v = scope->variables.find(name)->second;
            if(another_v.is_const) { throw std::runtime_error("Can't ReInitialize A Variable Marked As A Const \"" + name + "\""); };
            another_v.value = val;
            cache.insert({name, scope.get()});
        }
        auto& current_var = itr->second->variables.find(name)->second;
        if(current_var.is_const) { throw std::runtime_error("Can't ReInitialize A Variable Marked As A Const \"" + name + "\""); };
        current_var.value = val;
    }

    std::unordered_map<std::string, RunTimeVal*> flatten(bool copy = false){
        std::vector<std::pair<std::string, RunTimeVal*>> actual_vec;
        flatten_recursively(actual_vec);

        std::unordered_map<std::string, RunTimeVal*> hash_map;

        for(auto& [var_name, var_val] : actual_vec){
            if(!copy)
                hash_map.insert({std::move(var_name), var_val});
            else hash_map.insert({std::move(var_name), var_val->clone()});
        }

        return hash_map;
    }

    void flatten_recursively(std::vector<std::pair<std::string, RunTimeVal*>>& vec){
        for(const auto& [var_name, var_val] : variables) {
            vec.push_back({var_name, var_val.value});
        }
        if(parent){
            parent->flatten_recursively(vec);
        }
    }
    std::vector<RunTimeVal*> flatten_as_vec(){
        std::vector<RunTimeVal*> actual_vec;
        flatten_as_vec_recurse(actual_vec);
        return actual_vec;
    }

    void flatten_as_vec_recurse(std::vector<RunTimeVal*>& vec){
        for(const auto& [var_name, var_val] : variables) {
            vec.push_back(var_val.value);
        }
        if(parent){
            parent->flatten_as_vec_recurse(vec);
        }
    }
};

class SigmaInterpreter {
public:
    std::vector<RunTimeVal*> registered_event_handlers;
    std::unordered_map<std::string, RunTimeVal*> async_lambdas;
    std::vector<RunTimeVal*> wrapper_types_cache;
    std::string this_str = "this";
    Gtk::Window* current_window;
    PermissionContainer perms;

    std::string doc_name;

    DOMAccessor* accessor;
    static std::unordered_set<RunTimeValType> non_copyable_types;
    static std::unordered_map<RunTimeValType, std::string> type_to_string_table;

    std::shared_ptr<Scope> current_scope;
    std::unordered_set<RunTimeValType> break_out_types = {
        BreakType, ContinueType, ReturnType
    };
    std::unordered_map<std::string, std::vector<VariableDecleration*>> struct_decls;

    SigmaInterpreter();
    SigmaInterpreter(Gtk::Window* wind){
        current_window = wind;
    };
    
    void initialize();
    RunTimeVal* evaluate(Statement* stmt);
    RunTimeVal* evaluateProgram(SigmaProgram* program);
    RunTimeVal* evaluateBinaryExpression(BinaryExpression* expr);
    RunTimeVal* evaluateNumericBinaryExpression(NumVal* left,
        NumVal* right, std::string operat);
    RunTimeVal* evaluateBooleanBinaryExpression(BoolVal* left,
        BoolVal* right, std::string op);
    RunTimeVal* evaluateStringBinaryExpression(StringVal* left,
        StringVal* right, std::string op);
    RunTimeVal* evaluateFunctionCallExpression(FunctionCallExpression* expr);
    RunTimeVal* evaluateIndexAccessExpression(IndexAccessExpression* expr);
    RunTimeVal* evaluateMemberAccessExpression(MemberAccessExpression* expr);
    RunTimeVal* evaluateIncrementExpression(IncrementExpression* expr);
    RunTimeVal* evaluateDecrementExpression(DecrementExpression* expr);
    RunTimeVal* evaluateNegativeExpression(NegativeExpression* expr);
    RunTimeVal* evaluateJsObjectExpression(JsObjectExpression* expr);

    // Shadowing Is Allowed
    RunTimeVal* evaluateVariableDeclStatement(VariableDecleration* decl);
    RunTimeVal* evaluateVariableReInitStatement(VariableReInit* decl);
    RunTimeVal* evaluateIfStatement(IfStatement* if_Statement);
    RunTimeVal* evaluateWhileLoopStatement(WhileLoopStatement* while_loop);
    RunTimeVal* evaluateForLoopStatement(ForLoopStatement* for_loop);
    RunTimeVal* evaluateIndexReInitStatement(IndexReInitStatement* stmt);
    RunTimeVal* evaluateStructDeclStatement(StructDeclerationStatement* stmt);
    RunTimeVal* evaluateMemberReInitStatement(MemberReInitExpression* expr);
    RunTimeVal* evaluateCompoundAssignmentStatement(CompoundAssignmentStatement* stmt);


    RunTimeVal* toString(std::vector<RunTimeVal*>& args);
    RunTimeVal* numIota(std::vector<RunTimeVal*>& args);
    RunTimeVal* clone(std::vector<RunTimeVal*>& args);
    RunTimeVal* getCurrentTimeMillis(std::vector<RunTimeVal*>& args);
    RunTimeVal* getStringASCII(std::vector<RunTimeVal*>& args);
    RunTimeVal* toNum(std::vector<RunTimeVal*>& args);

    
    RunTimeVal* getElementById(std::vector<RunTimeVal*>& args);
    RunTimeVal* setElementInnerHtml(std::vector<RunTimeVal*>& args);
    RunTimeVal* getElementsByClassName(std::vector<RunTimeVal*>& args);
    RunTimeVal* setOnClick(std::vector<RunTimeVal*>& args);

    RunTimeVal* evaluateAnonymousLambdaCall(LambdaVal* lambda, std::vector<RunTimeVal*> args);

    static RunTimeVal* copyIfRecommended(RunTimeVal* val){
        if(non_copyable_types.contains(val->type) || !val->is_l_val)
            return val;
        return val->clone();
    }
    static bool shouldICopy(RunTimeVal* val){
        if(non_copyable_types.contains(val->type) || !val->is_l_val)
            return false;
        return true;
    }

    ObjectVal* getThis() {
        RunTimeVal* target_this = current_scope->getVal(this_str);
        if(target_this->type == StructType)
            return static_cast<ObjectVal*>(target_this);
        return nullptr;
    };

    std::vector<RunTimeVal*> getAccessibleValues();

};