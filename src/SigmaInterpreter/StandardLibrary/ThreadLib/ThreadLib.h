#pragma once
#include "../StdLib.h"
#include <memory>
#include <vector>

class ThreadLib {
public:
    static ObjectVal* getStruct();

    static RunTimeValue sleepCurrentThread(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue detachLambda(std::vector<RunTimeValue>& args, SigmaInterpreter*);
};