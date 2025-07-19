#include "Cryptography.h"
#include <cstring>
#include <iomanip>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <sstream>
#include <vector>

std::string Crypto::Sha256(std::string target_str) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(md_ctx, &target_str[0], target_str.size());
    unsigned int result_size;
    EVP_DigestFinal_ex(md_ctx, &result[0], &result_size);
    EVP_MD_CTX_free(md_ctx);

    std::ostringstream sstrea;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    return sstrea.str();
};

std::string Crypto::Sha512(std::string target_str){
    unsigned char result[SHA512_DIGEST_LENGTH];
    unsigned int result_size = 0;
    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(md_ctx, EVP_sha512(), nullptr);
    EVP_DigestUpdate(md_ctx, &target_str[0], target_str.size());
    EVP_DigestFinal_ex(md_ctx, result, &result_size);
    EVP_MD_CTX_free(md_ctx);

    std::ostringstream sstrea;
    for(int i = 0; i < SHA512_DIGEST_LENGTH; i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    return sstrea.str();
};

AesResult Crypto::Aes256(std::string target_str) {
    unsigned char key[32];
    unsigned char iv[16];

    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
        key, iv);
    std::vector<unsigned char> out(target_str.size() + 16);
    int len = 0;
    int cipher_len = 0;
    const unsigned char* actual_in = reinterpret_cast<const unsigned char*>(target_str.c_str());
    EVP_EncryptUpdate(ctx, out.data(), &len,  actual_in, target_str.size());
    cipher_len += len;
    EVP_EncryptFinal_ex(ctx, out.data() + len, &len);
    cipher_len += len;

    std::ostringstream sstrea;
    for(int i = 0; i < cipher_len; i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)out[i];
    }
    AesResult res = {sstrea.str(), 0};
    memcpy(res.key, key, 32);
    return res;
};