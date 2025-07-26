#include "CryptoLib.h"

StructVal* CryptoLib::getStruct(){
    std::unordered_map<std::string, RunTimeVal*> vals = {
        {"SHA256", RunTimeFactory::makeNativeFunction(&CryptoLib::Sha256Wrapper)},
        {"SHA512", RunTimeFactory::makeNativeFunction(&CryptoLib::Sha512Wrapper)},
        {"AES256Encrypt", RunTimeFactory::makeNativeFunction(&CryptoLib::Aes256Wrapper)},
        {"AES256Decrypt", RunTimeFactory::makeNativeFunction(&CryptoLib::Aes256DecryptWrapper)},
        {"AES256GenKey", RunTimeFactory::makeNativeFunction(&CryptoLib::Aes256GenKeyWrapper)},
    };

    return RunTimeFactory::makeStruct(std::move(vals));
};

RunTimeValue CryptoLib::Sha256Wrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::Sha256(dynamic_cast<StringVal*>(args[0])->str));
};

RunTimeValue CryptoLib::Sha512Wrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::Sha512(dynamic_cast<StringVal*>(args[0])->str));
};
RunTimeValue CryptoLib::Aes256Wrapper(std::vector<RunTimeValue>& args) {
    std::vector<unsigned char> res = Crypto::Aes256(dynamic_cast<StringVal*>(args[0])->str,
        dynamic_cast<BinaryVal*>(args[1])->binary_data);
    return RunTimeFactory::makeBinary(res);
};

RunTimeValue CryptoLib::Aes256GenKeyWrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeBinary(Crypto::genAes256Key());
};

RunTimeValue CryptoLib::Aes256DecryptWrapper(std::vector<RunTimeValue>& args) {
    return RunTimeFactory::makeString(Crypto::decryptAes256(
        dynamic_cast<BinaryVal*>(args[0])->binary_data,
         dynamic_cast<BinaryVal*>(args[1])->binary_data));
};