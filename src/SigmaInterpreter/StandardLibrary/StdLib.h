#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../RunTime.h"

class Scope;
typedef RunTimeVal* RunTimeValue;
typedef std::unordered_map<std::string, RunTimeVal*> LibMapType;

#define COMPILED_FUNC_ARGS std::vector<RunTimeValue>& args, SigmaInterpreter* interpreter

class StdLib {
public:
    static std::shared_ptr<Scope> current_calling_scope;
    void addValToStruct(ObjectVal* target_struct, std::string name,
        RunTimeVal* val);

};

class StdLibInitializer {
    void declareStdLibMemberInScope(std::string struct_name, RunTimeVal* struct_val,
        Scope* target_scope);
};