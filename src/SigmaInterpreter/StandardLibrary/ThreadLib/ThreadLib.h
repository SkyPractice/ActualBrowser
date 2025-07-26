#pragma once
#include "../StdLib.h"
#include <memory>
#include <vector>

class ThreadLib {
public:
    static StructVal* getStruct();

    static RunTimeValue sleepCurrentThread(std::vector<RunTimeValue>& args);
    static RunTimeValue detachLambda(std::vector<RunTimeValue>& args);
};