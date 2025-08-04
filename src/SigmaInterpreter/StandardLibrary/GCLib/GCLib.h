#pragma once
#include "../StdLib.h"

class GCLib {
public:
    static ObjectVal* getStruct();

    static RunTimeVal* mark(COMPILED_FUNC_ARGS);
    static RunTimeVal* sweep(COMPILED_FUNC_ARGS);
};