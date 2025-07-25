#pragma once
#include "../StdLib.h"

class ArrayLib {
public:
    static std::shared_ptr<StructVal> getStruct();

    static RunTimeValue resizeArray(std::vector<RunTimeValue>& args);
    static RunTimeValue pushBackArray(std::vector<RunTimeValue>& args);
    static RunTimeValue popBackArray(std::vector<RunTimeValue>& args);
    static RunTimeValue pushFirstArray(std::vector<RunTimeValue>& args);
    static RunTimeValue popFirstArray(std::vector<RunTimeValue>& args);
    static RunTimeValue insertIntoArray(std::vector<RunTimeValue>& args);
    static RunTimeValue arraySize(std::vector<RunTimeValue>& args);
};