#include "Scope.h"

std::shared_ptr<Scope> Scope::traverse(const std::string& var_name){
    if(variables.contains(var_name)) return shared_from_this();
    else if (parent) return parent->traverse(var_name);
    return nullptr;
};
RunTimeVal* Scope::getVal(std::string& var_name){
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
void Scope::declareVar(std::string name, Variable val){
    if(traverse(name))
        cache.clear();
    val.value->is_l_val = true;
    variables[name] = val;
};
void Scope::reInitVar(std::string& name, RunTimeVal* val){
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
std::unordered_map<std::string, RunTimeVal*> Scope::flatten(bool copy){
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
void Scope::flatten_recursively(std::vector<std::pair<std::string, RunTimeVal*>>& vec){
    for(const auto& [var_name, var_val] : variables) {
        vec.push_back({var_name, var_val.value});
    }
    if(parent){
        parent->flatten_recursively(vec);
    }
}
std::vector<RunTimeVal*> Scope::flatten_as_vec(){
    std::vector<RunTimeVal*> actual_vec;
    flatten_as_vec_recurse(actual_vec);
    return actual_vec;
}
void Scope::flatten_as_vec_recurse(std::vector<RunTimeVal*>& vec){
    for(const auto& [var_name, var_val] : variables) {
        vec.push_back(var_val.value);
    }
    if(parent){
        parent->flatten_as_vec_recurse(vec);
    }
}