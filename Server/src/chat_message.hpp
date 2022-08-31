#pragma once
#include <cstring>
#include <vector>
#include <iostream>
#include "inet.hpp"

using msg_len_t = uint32_t;

struct chat_message 
{
    chat_message()
    : body_len{ 0 }
    , msg_data(header_len + 16382)
    { max_body_len = 16382; }

    chat_message(size_t max_len)
    : body_len{ 0 }
    , msg_data( header_len + max_len )
    { chat_message::max_body_len = max_len; }

    static size_t header_length() { return header_len; }

    char const * data() const { return msg_data.data(); }

    char* data() { return &msg_data[0]; }

    char const * body() const { return msg_data.data() + header_len; }

    char* body() { return &msg_data[0] + header_len; }

    size_t body_length() const { return body_len; }

    size_t length() const { return header_len + body_len; }

    void body_length(msg_len_t new_len) { body_len = (new_len > max_body_len) ? max_body_len : new_len; }

    bool decode_header();

    void encode_header();

    bool from_str(uint8_t msg_type, char const *str, size_t len);

private:

    msg_len_t decode_body_len();
    
    inline static msg_len_t const header_len = sizeof(msg_len_t);
    inline static msg_len_t       max_body_len;
                  msg_len_t       body_len;
           std::vector<char>      msg_data;
};