#pragma once
#include <string>

struct AesResult {
    std::string cipher;
    unsigned char key[32];
};

class Crypto {
public:
    static std::string Sha256(std::string target_str);
    static std::string Sha512(std::string target_str);
    static AesResult Aes256(std::string target_str);
};