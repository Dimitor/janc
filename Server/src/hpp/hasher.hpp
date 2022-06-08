#pragma once
#include <vector>
#include <sstream>
#include <iomanip>
#include <utility>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

//class for hashing strings
struct hasher
{
    static std::string sha3_512(std::string const &input);
    
    static std::string sha3_512(char const *input_data, size_t input_len);

    static std::pair<std::string, std::string> sha3_512_with_salt(std::string const &input);
    
    static std::string sha3_512_with_salt(std::string const &input, std::string const &salt);

private:

    static std::string bytes_to_hex_string(std::vector<uint8_t> const &bytes);
};