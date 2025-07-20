#include "Cryptography.h"
#include <cstring>
#include <iomanip>
#include <openssl/bn.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/types.h>
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

std::vector<unsigned char> Crypto::Aes256(std::string target_str, std::vector<unsigned char> key) {
    std::vector<unsigned char> iv(16);

    RAND_bytes(iv.data(), 16);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
        key.data(), iv.data());
    std::vector<unsigned char> out(target_str.size() + 16);
    int len = 0;
    int cipher_len = 0;
    const unsigned char* actual_in = reinterpret_cast<const unsigned char*>(target_str.c_str());
    EVP_EncryptUpdate(ctx, out.data(), &len,  actual_in, target_str.size());
    cipher_len += len;
    EVP_EncryptFinal_ex(ctx, out.data() + len, &len);
    cipher_len += len;

    out.resize(cipher_len);
    out.insert(out.begin(), iv.begin(), iv.end());

    return out;
};

std::string Crypto::decryptAes256(std::vector<unsigned char> cipher, std::vector<unsigned char> key){
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), cipher.data());
    int out_len = 0;
    int total_len = 0;
    std::vector<unsigned char> result(cipher.size() + 16);
    EVP_DecryptUpdate(ctx, result.data(), &out_len, cipher.data() + 16, cipher.size() - 16);
    total_len += out_len;
    EVP_DecryptFinal_ex(ctx, result.data() + out_len, &out_len);
    total_len += out_len;
    EVP_CIPHER_CTX_free(ctx);
    
    std::string str(reinterpret_cast<const char*>(result.data()), total_len);
    
    return str;
}

RsaResult Crypto::Rsa(std::string target_str) {
    RSA* rs = RSA_new();
    EVP_PKEY* key = EVP_PKEY_new();
    BIGNUM* num = BN_new();
    BN_set_word(num, RSA_F4);

    RSA_generate_key_ex(rs, 2048, num, NULL);
    EVP_PKEY_assign_RSA(key, rs);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(key, NULL);
    EVP_PKEY_encrypt_init(ctx);
    EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING);
    size_t out_len = 0;
    const unsigned char* actual_in = (const unsigned char*) target_str.c_str();
    EVP_PKEY_encrypt(ctx, NULL, &out_len, actual_in, target_str.size());
    std::vector<unsigned char> result(out_len);
    EVP_PKEY_encrypt(ctx, result.data(), &out_len, actual_in, target_str.size());

    BN_free(num);
    EVP_PKEY_free(key);
    RSA_free(rs);
    EVP_PKEY_CTX_free(ctx);

    std::ostringstream sstrea;
    for(int i = 0; i < out_len; i++){
        sstrea << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }

    return { sstrea.str() };
};

std::vector<unsigned char> Crypto::genAes256Key() {
    std::vector<unsigned char> key(32);
    RAND_bytes(key.data(), 32);

    return key;
};