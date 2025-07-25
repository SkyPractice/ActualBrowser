#pragma once
#include "../StdLib.h"

class FilesLib : public StdLib {
public:
    static std::shared_ptr<StructVal> getStruct();

    static RunTimeValue readFileSync(std::vector<RunTimeValue>& args);
    static RunTimeValue writeFileSync(std::vector<RunTimeValue>& args);
    static RunTimeValue writeBinaryFileSync(std::vector<RunTimeValue>& args);
    static RunTimeValue readBinaryFileSync(std::vector<RunTimeValue>& args);
    static RunTimeValue readFileAsync(std::vector<RunTimeValue>& args);
    static RunTimeValue writeFileAsync(std::vector<RunTimeValue>& args);
    static RunTimeValue writeBinaryFileAsync(std::vector<RunTimeValue>& args);
    static RunTimeValue readBinaryFileAsync(std::vector<RunTimeValue>& args);
};