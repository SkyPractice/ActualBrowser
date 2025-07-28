#pragma once
#include "../StdLib.h"

class MathLib {
public:
    static RunTimeVal* getStruct();

    static RunTimeVal* mathSqrt(COMPILED_FUNC_ARGS);
    static RunTimeVal* mathPow(COMPILED_FUNC_ARGS);
    static RunTimeVal* mathRand(COMPILED_FUNC_ARGS);
};