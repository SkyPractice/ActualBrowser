#pragma once
#include "../StdLib.h"

// Compiled Functions Are Deprecated, Gonna Be Replaced By An IsArray Function . . . 
class ArrayLib {
public:
    static ObjectVal* getStruct();

    static RunTimeValue resizeArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue pushBackArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue popBackArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue pushFirstArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue popFirstArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue insertIntoArray(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue arraySize(std::vector<RunTimeValue>& args, SigmaInterpreter*);
};