#pragma once
#include "../StdLib.h"

class CryptoLib : public StdLib {
public:
    static ObjectVal* getStruct();

    static RunTimeValue Sha256Wrapper(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue Sha512Wrapper(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue Aes256Wrapper(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue Aes256DecryptWrapper(std::vector<RunTimeValue>& args, SigmaInterpreter*);
    static RunTimeValue Aes256GenKeyWrapper(std::vector<RunTimeValue>& args, SigmaInterpreter*);
};