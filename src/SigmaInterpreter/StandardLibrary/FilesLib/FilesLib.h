#pragma once
#include "../StdLib.h"

class FilesLib : public StdLib {
public:
    static ObjectVal* getStruct();

    static RunTimeValue readFileSync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue writeFileSync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue writeBinaryFileSync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue readBinaryFileSync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue readFileAsync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue writeFileAsync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue writeBinaryFileAsync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue readBinaryFileAsync(std::vector<RunTimeValue>& args, SigmaInterpreter*);
};