#include "../hpp/hasher.hpp"

std::string hasher::bytes_to_hex_string(std::vector<uint8_t> const &bytes)
{
    std::ostringstream stream;
    for (uint8_t b : bytes)
    {
        stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(b);
    }
    return stream.str();
}

std::pair<std::string, std::string> hasher::sha3_512_with_salt(std::string const &input)
{
    std::vector<uint8_t> bytes(64);
    RAND_bytes(bytes.data(), 64);
    
    auto salt = bytes_to_hex_string(bytes);

    std::string password_hash = sha3_512_with_salt(input, salt);
    return std::make_pair(salt, password_hash);
}

std::string hasher::sha3_512_with_salt(std::string const &input, 
                                       std::string const &salt)
{
    std::string input_with_salt = input + salt;

    return sha3_512(std::move(input_with_salt));
}

std::string hasher::sha3_512(std::string const &input)
{
    return sha3_512(input.c_str(), input.size());
}

std::string hasher::sha3_512(char const *input_data, size_t input_len)
{
    EVP_MD const * algorithm = EVP_sha3_512();

    uint32_t digest_length = SHA512_DIGEST_LENGTH;
    uint8_t *digest        = static_cast<uint8_t*>(OPENSSL_malloc(digest_length));
    
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    
    EVP_DigestInit_ex(context, algorithm, nullptr);
    EVP_DigestUpdate(context, input_data, input_len);
    EVP_DigestFinal_ex(context, digest, &digest_length);
    EVP_MD_CTX_destroy(context);
    
    std::string hash = bytes_to_hex_string(std::vector<uint8_t>(digest, digest + digest_length));

    OPENSSL_free(digest);
    
    return hash;
}