#pragma once
#include <string>
#include <vector>


struct RsaResult {
    std::string cipher;
};

class Crypto {
public:
    static std::string Sha256(std::string target_str);
    static std::string Sha512(std::string target_str);
    static std::vector<unsigned char> Aes256(std::string target_str, std::vector<unsigned char> key);
    static std::string decryptAes256(std::vector<unsigned char> cipher, std::vector<unsigned char> key);
    static std::vector<unsigned char> genAes256Key();
    static RsaResult Rsa(std::string target_str);
};