#pragma once
#include "RunTime.h"
#include <memory>
#include <unordered_map>

struct Variable {
    RunTimeVal* value;
    bool is_const;
};

class Scope : public std::enable_shared_from_this<Scope>{
public: 
    std::shared_ptr<Scope> parent;
    std::unordered_map<std::string, Variable> variables;
    std::unordered_map<std::string, Scope*> cache;

    Scope(std::shared_ptr<Scope> p): parent(p) {};

    std::shared_ptr<Scope> traverse(const std::string& var_name);

    RunTimeVal* getVal(std::string& var_name);

    // shadowing is allowed
    void declareVar(std::string name, Variable val);

    void reInitVar(std::string& name, RunTimeVal* val);

    std::unordered_map<std::string, RunTimeVal*> flatten(bool copy = false);
    void flatten_recursively(std::vector<std::pair<std::string, RunTimeVal*>>& vec);
    
    
    std::vector<RunTimeVal*> flatten_as_vec();
    void flatten_as_vec_recurse(std::vector<RunTimeVal*>& vec);
};