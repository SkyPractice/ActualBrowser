#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "../RunTime.h"

class Scope;

class StdLib {
public:
    void addValToStruct(std::shared_ptr<StructVal> target_struct, std::string name,
        std::shared_ptr<RunTimeVal> val);

    static RunTimeValue copyIfRecommended(RunTimeValue val){
        if(val->type == StructType || val->type == LambdaType || val->type == StringType ||
            val->type == ArrayType || val->type == BinaryType || val->type == NativeFunctionType ||
            val->type == HtmlType)
            return val;
        return val->clone();
    }
    static bool shouldICopy(RunTimeValue val){
        if(val->type == StructType || val->type == LambdaType || val->type == StringType ||
            val->type == ArrayType || val->type == BinaryType || val->type == NativeFunctionType ||
            val->type == HtmlType)
            return false;
        return true;
    }
};

class StdLibInitializer {
    void declareStdLibMemberInScope(std::string struct_name, std::shared_ptr<RunTimeVal> struct_val,
        std::shared_ptr<Scope> target_scope);
};