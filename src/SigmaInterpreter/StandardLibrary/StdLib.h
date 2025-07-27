#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../RunTime.h"

class Scope;
typedef RunTimeVal* RunTimeValue;

#define COMPILED_FUNC_ARGS std::vector<RunTimeValue>& args, SigmaInterpreter* interpreter

class StdLib {
public:
    static std::shared_ptr<Scope> current_calling_scope;
    void addValToStruct(StructVal* target_struct, std::string name,
        RunTimeVal* val);

    static RunTimeVal* copyIfRecommended(RunTimeVal* val){
        if(val->type == StructType || val->type == LambdaType || val->type == StringType ||
            val->type == ArrayType || val->type == BinaryType || val->type == NativeFunctionType ||
            val->type == HtmlType)
            return val;
        return val->clone();
    }
    static bool shouldICopy(RunTimeVal* val){
        if(val->type == StructType || val->type == LambdaType || val->type == StringType ||
            val->type == ArrayType || val->type == BinaryType || val->type == NativeFunctionType ||
            val->type == HtmlType)
            return false;
        return true;
    }
};

class StdLibInitializer {
    void declareStdLibMemberInScope(std::string struct_name, RunTimeVal* struct_val,
        Scope* target_scope);
};