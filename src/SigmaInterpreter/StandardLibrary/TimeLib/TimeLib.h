#pragma once
#include "../StdLib.h"

class TimeLib {
public:
    static RunTimeVal* getStruct();

    static RunTimeVal* getCurrentTimeMillis(COMPILED_FUNC_ARGS);
    static RunTimeVal* getCurrentTimeMicros(COMPILED_FUNC_ARGS);
    static RunTimeVal* getCurrentTimeSecs(COMPILED_FUNC_ARGS);
    static RunTimeVal* getTime(COMPILED_FUNC_ARGS);
};