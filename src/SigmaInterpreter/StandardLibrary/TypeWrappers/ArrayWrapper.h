#pragma once
#include "../StdLib.h"
#include <unordered_map>

class ArrayWrapper {
public:
    static ObjectVal* genObject(ArrayVal* array);
    static std::unordered_map<std::string, RunTimeVal*> funcs;

    static void initializeWrapper();
    static RunTimeVal* get(COMPILED_FUNC_ARGS);
    static RunTimeVal* set(COMPILED_FUNC_ARGS);
    static RunTimeVal* push(COMPILED_FUNC_ARGS);
    static RunTimeVal* pop(COMPILED_FUNC_ARGS);
    static RunTimeVal* filter(COMPILED_FUNC_ARGS);
    static RunTimeVal* forEach(COMPILED_FUNC_ARGS);
    static RunTimeVal* reverse(COMPILED_FUNC_ARGS);
    static RunTimeVal* at(COMPILED_FUNC_ARGS);
    static RunTimeVal* length(COMPILED_FUNC_ARGS);
    static RunTimeVal* map(COMPILED_FUNC_ARGS);
    static RunTimeVal* indexOf(COMPILED_FUNC_ARGS);
    static RunTimeVal* contains(COMPILED_FUNC_ARGS);
    static RunTimeVal* pushFirst(COMPILED_FUNC_ARGS);
    static RunTimeVal* popFirst(COMPILED_FUNC_ARGS);
    static RunTimeVal* insert(COMPILED_FUNC_ARGS);
    static RunTimeVal* clear(COMPILED_FUNC_ARGS);
    static RunTimeVal* back(COMPILED_FUNC_ARGS);
    static RunTimeVal* front(COMPILED_FUNC_ARGS);
    static RunTimeVal* resize(COMPILED_FUNC_ARGS);
    static RunTimeVal* slice(COMPILED_FUNC_ARGS);
};