#pragma once
#include "../StdLib.h"

class StringWrapper {
public:
    static ObjectVal* genObject(StringVal* str_val);
    static std::unordered_map<std::string, RunTimeVal*> funcs;

    static void initializeWrapper();

    static RunTimeVal* append(COMPILED_FUNC_ARGS);
    static RunTimeVal* push(COMPILED_FUNC_ARGS);
    static RunTimeVal* pop(COMPILED_FUNC_ARGS);
    static RunTimeVal* get(COMPILED_FUNC_ARGS);
    static RunTimeVal* set(COMPILED_FUNC_ARGS);
    static RunTimeVal* indexOf(COMPILED_FUNC_ARGS);
    static RunTimeVal* indexOfChar(COMPILED_FUNC_ARGS);
    static RunTimeVal* length(COMPILED_FUNC_ARGS);
    static RunTimeVal* forEach(COMPILED_FUNC_ARGS);
    static RunTimeVal* reverse(COMPILED_FUNC_ARGS);
    static RunTimeVal* contains(COMPILED_FUNC_ARGS);
    static RunTimeVal* containsChar(COMPILED_FUNC_ARGS);
    static RunTimeVal* front(COMPILED_FUNC_ARGS);
    static RunTimeVal* back(COMPILED_FUNC_ARGS);
    static RunTimeVal* insert(COMPILED_FUNC_ARGS);
    static RunTimeVal* insertChar(COMPILED_FUNC_ARGS);
    static RunTimeVal* clear(COMPILED_FUNC_ARGS);
    static RunTimeVal* at(COMPILED_FUNC_ARGS);
    static RunTimeVal* erase(COMPILED_FUNC_ARGS);
    static RunTimeVal* substr(COMPILED_FUNC_ARGS);
    static RunTimeVal* getCharArray(COMPILED_FUNC_ARGS);
};