#include "ArrayWrapper.h"
#include "../../SigmaInterpreter.h"
#include <unordered_map>

std::unordered_map<std::string, RunTimeVal*> ArrayWrapper::funcs = {};

void ArrayWrapper::initializeWrapper() {
    funcs = {
        {"get", RunTimeFactory::makeNativeFunction(&ArrayWrapper::get, {})},
        {"set", RunTimeFactory::makeNativeFunction(&ArrayWrapper::set, {})},
        {"push", RunTimeFactory::makeNativeFunction(&ArrayWrapper::push, {{"val", AnyType}})},
        {"pop", RunTimeFactory::makeNativeFunction(&ArrayWrapper::pop, {})},
        {"pushFirst", RunTimeFactory::makeNativeFunction(&ArrayWrapper::pushFirst, {{"val", AnyType}})},
        {"popFirst", RunTimeFactory::makeNativeFunction(&ArrayWrapper::popFirst, {})},
        {"length", RunTimeFactory::makeNativeFunction(&ArrayWrapper::length, {})},
        {"filter", RunTimeFactory::makeNativeFunction(&ArrayWrapper::filter, {{"predicate", LambdaType}})},
        {"forEach", RunTimeFactory::makeNativeFunction(&ArrayWrapper::forEach, {{"function", LambdaType}})},
        {"reverse", RunTimeFactory::makeNativeFunction(&ArrayWrapper::reverse, {})},
       {"at", RunTimeFactory::makeNativeFunction(&ArrayWrapper::at, {{"index", NumType}})},
       {"map", RunTimeFactory::makeNativeFunction(&ArrayWrapper::map, {{"function", LambdaType}})},
       {"indexOf", RunTimeFactory::makeNativeFunction(&ArrayWrapper::indexOf, {{"predicate", LambdaType}})},
       {"contains", RunTimeFactory::makeNativeFunction(&ArrayWrapper::contains, {{"predicate", LambdaType}})},
       {"insert", RunTimeFactory::makeNativeFunction(&ArrayWrapper::insert, {{"index", NumType}, {"val", AnyType}})},
       {"clear", RunTimeFactory::makeNativeFunction(&ArrayWrapper::clear, {})},
       {"resize", RunTimeFactory::makeNativeFunction(&ArrayWrapper::resize, {{"new_size", NumType}})},
       {"slice", RunTimeFactory::makeNativeFunction(&ArrayWrapper::slice, {{"starting_index", NumType}, {"size", NumType}})},
       {"is_primitive", RunTimeFactory::makeString("array")}
    };
};

ObjectVal* ArrayWrapper::genObject(ArrayVal* array) {
    std::unordered_map<std::string, RunTimeVal*> vals = {
    {"primitive", array},    
    };

    for(auto& func : funcs){
        vals.insert(func);
    }

    return RunTimeFactory::makeStruct(vals);
};

RunTimeVal* ArrayWrapper::get(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    return object->vals["primitive"];
};
RunTimeVal* ArrayWrapper::set(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    object->vals["primitive"] = args[0];
    return args[0];
};

RunTimeVal* ArrayWrapper::push(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    primitive->vals.push_back(interpreter->copyIfRecommended(args[0]));
    return nullptr;
};
RunTimeVal* ArrayWrapper::pop(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    primitive->vals.pop_back();
    return nullptr;
};
RunTimeVal* ArrayWrapper::filter(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    ArrayVal* new_ret_array = RunTimeFactory::makeArray({});
    LambdaVal* predicate = static_cast<LambdaVal*>(args[0]);

    for(auto& val : primitive->vals) {
        RunTimeVal* res = interpreter->evaluateAnonymousLambdaCall(predicate, {val});
        BoolVal* result = static_cast<BoolVal*>(res);
        if(result->boolean)
            new_ret_array->vals.push_back(val);
    }

    return ArrayWrapper::genObject(new_ret_array);
};
RunTimeVal* ArrayWrapper::forEach(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    LambdaVal* call = static_cast<LambdaVal*>(args[1]);

    for(auto& val : primitive->vals){
        interpreter->evaluateAnonymousLambdaCall(call, {val});
    }

    return nullptr;
};
RunTimeVal* ArrayWrapper::reverse(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    std::reverse(primitive->vals.begin(), primitive->vals.end());

    return nullptr;
};
RunTimeVal* ArrayWrapper::at(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    return primitive->vals.at((size_t)static_cast<NumVal*>(args[0])->num);
};

RunTimeVal* ArrayWrapper::length(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    return RunTimeFactory::makeNum(primitive->vals.size());
};

RunTimeVal* ArrayWrapper::map(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    LambdaVal* function = static_cast<LambdaVal*>(args[0]);
    ArrayVal* new_arr = RunTimeFactory::makeArray({});

    for(auto& val : primitive->vals){
        new_arr->vals.push_back(interpreter->evaluateAnonymousLambdaCall(function,
            {val->clone()}));
    }

    return ArrayWrapper::genObject(new_arr);
}

RunTimeVal* ArrayWrapper::indexOf(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    LambdaVal* matchingLambda = static_cast<LambdaVal*>(args[0]);

    for(int i = 0; i < primitive->vals.size(); i++){
        RunTimeVal* result = interpreter->evaluateAnonymousLambdaCall(matchingLambda,
            {primitive->vals[i]});
        if(result->type == BoolType){
            BoolVal* bool_result = static_cast<BoolVal*>(result);
            if(bool_result->boolean){
                return RunTimeFactory::makeNum(i);
            }
        }
    }

    return RunTimeFactory::makeNum(-1);
};
RunTimeVal* ArrayWrapper::contains(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    LambdaVal* matchingLambda = static_cast<LambdaVal*>(args[0]);

    for(int i = 0; i < primitive->vals.size(); i++){
        RunTimeVal* result = interpreter->evaluateAnonymousLambdaCall(matchingLambda,
            {primitive->vals[i]});
        if(result->type == BoolType){
            BoolVal* bool_result = static_cast<BoolVal*>(result);
            if(bool_result->boolean){
                return RunTimeFactory::makeBool(true);
            }
        }
    }
    return RunTimeFactory::makeBool(false);
};
RunTimeVal* ArrayWrapper::pushFirst(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    RunTimeVal* val = args[0];

    primitive->vals.insert(primitive->vals.begin(), interpreter->copyIfRecommended(val));

    return nullptr;
};
RunTimeVal* ArrayWrapper::popFirst(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    primitive->vals.erase(primitive->vals.begin());

    return nullptr;
};
RunTimeVal* ArrayWrapper::insert(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    NumVal* index = static_cast<NumVal*>(args[0]);
    RunTimeVal* val = args[1];

    primitive->vals.insert(primitive->vals.begin() + (size_t)index->num, val);

    return nullptr;
};

RunTimeVal* ArrayWrapper::clear(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    primitive->vals.clear();

    return nullptr;
};
RunTimeVal* ArrayWrapper::back(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    return primitive->vals.back();
};
RunTimeVal* ArrayWrapper::front(COMPILED_FUNC_ARGS){
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);

    return primitive->vals.front();
};
RunTimeVal* ArrayWrapper::resize(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    NumVal* new_size = static_cast<NumVal*>(args[0]);
    
    primitive->vals.resize((size_t)new_size->num);
    return nullptr;
};
RunTimeVal* ArrayWrapper::slice(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    ArrayVal* primitive = static_cast<ArrayVal*>(object->vals["primitive"]);
    NumVal* start_index = static_cast<NumVal*>(args[0]);
    NumVal* size = static_cast<NumVal*>(args[1]);
    ArrayVal* new_array = RunTimeFactory::makeArray({});

    for(size_t i = start_index->num; i < start_index->num + size->num; i++){
        new_array->vals.push_back(primitive->vals[i]->clone());
    }

    return ArrayWrapper::genObject(new_array);
};