#include "StringWrapper.h"
#include "../../SigmaInterpreter.h"
#include "ArrayWrapper.h"
#include <algorithm>
#include <string>

ObjectVal* StringWrapper::genObject(StringVal* str_val) {
    std::unordered_map<std::string, RunTimeVal*> vals = {
        {"primitive", str_val},    
    };

    for(auto& func : funcs){
        vals.insert(func);
    }

    return RunTimeFactory::makeStruct(vals);
};

std::unordered_map<std::string, RunTimeVal*> StringWrapper::funcs = {};

void StringWrapper::initializeWrapper() {
    funcs = {
        {"get", RunTimeFactory::makeNativeFunction(&StringWrapper::get, {})},
        {"set", RunTimeFactory::makeNativeFunction(&StringWrapper::set, {})},
        {"push", RunTimeFactory::makeNativeFunction(&StringWrapper::push, {{"val", CharType}})},
        {"pop", RunTimeFactory::makeNativeFunction(&StringWrapper::pop, {})},
        {"length", RunTimeFactory::makeNativeFunction(&StringWrapper::length, {})},
        {"forEach", RunTimeFactory::makeNativeFunction(&StringWrapper::forEach, {{"function", LambdaType}})},
        {"reverse", RunTimeFactory::makeNativeFunction(&StringWrapper::reverse, {})},
       {"at", RunTimeFactory::makeNativeFunction(&StringWrapper::at, {{"index", NumType}})},
       {"indexOf", RunTimeFactory::makeNativeFunction(&StringWrapper::indexOf, {{"str", StringType}})},
       {"contains", RunTimeFactory::makeNativeFunction(&StringWrapper::contains, {{"str", StringType}})},
       {"containsChar", RunTimeFactory::makeNativeFunction(&StringWrapper::contains, {{"char", CharType}})},
       {"insert", RunTimeFactory::makeNativeFunction(&StringWrapper::insert, {{"index", NumType}, {"val", StringType}})},
       {"clear", RunTimeFactory::makeNativeFunction(&StringWrapper::clear, {})},
       {"substr", RunTimeFactory::makeNativeFunction(&StringWrapper::substr, {{"starting_index", NumType}, {"size", NumType}})},
       {"insertChar", RunTimeFactory::makeNativeFunction(&StringWrapper::insertChar, {{"index", NumType}, {"char", CharType}})},
       {"indexOfChar", RunTimeFactory::makeNativeFunction(&StringWrapper::indexOfChar, {{"char", CharType}})},
       {"toCharArray", RunTimeFactory::makeNativeFunction(&StringWrapper::getCharArray, {})},
       {"erase", RunTimeFactory::makeNativeFunction(&StringWrapper::erase, {{"index", NumType}, {"size", NumType}})},
       {"is_primitive", RunTimeFactory::makeString("string")}
    };
};
RunTimeVal* StringWrapper::append(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    StringVal* target_str = static_cast<StringVal*>(args[0]);

    primitive->str.append(target_str->str);

    return nullptr;
};
RunTimeVal* StringWrapper::push(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    CharVal* target_char = static_cast<CharVal*>(args[0]);

    primitive->str.push_back(target_char->ch);

    return nullptr;
};
RunTimeVal* StringWrapper::pop(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    primitive->str.pop_back();

    return nullptr;
};
RunTimeVal* StringWrapper::get(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    return object->vals["primitive"];
};
RunTimeVal* StringWrapper::set(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    object->vals["primitive"] = args[0];

    return nullptr;
};
RunTimeVal* StringWrapper::indexOf(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    StringVal* target_str = static_cast<StringVal*>(args[0]);

    size_t result = primitive->str.find(target_str->str);
    if(result == std::string::npos){
        return RunTimeFactory::makeNum(-1);
    }

    return RunTimeFactory::makeNum(result);
};
RunTimeVal* StringWrapper::length(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    return RunTimeFactory::makeNum(primitive->str.size());
};
RunTimeVal* StringWrapper::forEach(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    LambdaVal* function = static_cast<LambdaVal*>(args[0]);

    for(auto& ch : primitive->str) {
        interpreter->evaluateAnonymousLambdaCall(function, {RunTimeFactory::
            makeChar(ch)});
    }

    return nullptr;
};
RunTimeVal* StringWrapper::reverse(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    std::reverse(primitive->str.begin(), primitive->str.end());

    return nullptr;
};
RunTimeVal* StringWrapper::contains(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    StringVal* target_str = static_cast<StringVal*>(args[0]);

    if(primitive->str.find(target_str->str) != std::string::npos)
        return RunTimeFactory::makeBool(true);
    return RunTimeFactory::makeBool(false);
};
RunTimeVal* StringWrapper::containsChar(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    CharVal* target_char = static_cast<CharVal*>(args[0]);

    if(primitive->str.find(target_char->ch) != std::string::npos)
        return RunTimeFactory::makeBool(true);
    return RunTimeFactory::makeBool(false);
};
RunTimeVal* StringWrapper::front(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    return RunTimeFactory::makeChar(primitive->str.front());
};
RunTimeVal* StringWrapper::back(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    return RunTimeFactory::makeChar(primitive->str.back());
};
RunTimeVal* StringWrapper::insert(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    NumVal* index = static_cast<NumVal*>(args[0]);
    StringVal* target_str = static_cast<StringVal*>(args[1]);

    primitive->str.insert(primitive->str.begin() + (size_t)index->num, target_str->str.begin(),
        target_str->str.end());

    return nullptr;
};
RunTimeVal* StringWrapper::insertChar(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    NumVal* index = static_cast<NumVal*>(args[0]);
    CharVal* target_char = static_cast<CharVal*>(args[1]);

    primitive->str.insert(primitive->str.begin() + (size_t)index, target_char->ch);

    return nullptr;
};
RunTimeVal* StringWrapper::clear(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);

    primitive->str.clear();

    return nullptr;
};
RunTimeVal* StringWrapper::at(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    NumVal* index = static_cast<NumVal*>(args[0]);

    return RunTimeFactory::makeChar(primitive->str[(size_t)index->num]);
};
RunTimeVal* StringWrapper::erase(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    NumVal* starting_pos = static_cast<NumVal*>(args[0]);
    NumVal* size = static_cast<NumVal*>(args[1]);

    primitive->str.erase((size_t)starting_pos->num, (size_t)size->num);

    return nullptr;
};
RunTimeVal* StringWrapper::substr(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    NumVal* starting_pos = static_cast<NumVal*>(args[0]);
    NumVal* size = static_cast<NumVal*>(args[1]);

    return  StringWrapper::genObject(RunTimeFactory::makeString(
        primitive->str.substr((size_t)starting_pos->num, (size_t)size->num)
    ));
};
RunTimeVal* StringWrapper::getCharArray(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    ArrayVal* new_array = RunTimeFactory::makeArray({});

    for(auto& ch : primitive->str){
        new_array->vals.push_back(RunTimeFactory::makeChar(ch));
    }

    return ArrayWrapper::genObject(new_array);
};
RunTimeVal* StringWrapper::indexOfChar(COMPILED_FUNC_ARGS) {
    ObjectVal* object = interpreter->getThis();
    StringVal* primitive = static_cast<StringVal*>(object->vals["primitive"]);
    CharVal* target_char = static_cast<CharVal*>(args[0]);

    size_t index = primitive->str.find(target_char->ch);
    if(index == std::string::npos)
        return RunTimeFactory::makeNum(-1);

    return RunTimeFactory::makeNum((double)index);
};