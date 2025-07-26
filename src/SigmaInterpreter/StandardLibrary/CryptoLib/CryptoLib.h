#pragma once
#include "../StdLib.h"
#include "../../Cryptography.h"

class CryptoLib : public StdLib {
public:
    static StructVal* getStruct();

    static RunTimeValue Sha256Wrapper(std::vector<RunTimeValue>& args);
    static RunTimeValue Sha512Wrapper(std::vector<RunTimeValue>& args);
    static RunTimeValue Aes256Wrapper(std::vector<RunTimeValue>& args);
    static RunTimeValue Aes256DecryptWrapper(std::vector<RunTimeValue>& args);
    static RunTimeValue Aes256GenKeyWrapper(std::vector<RunTimeValue>& args);
};