#include "../hpp/chat_message.hpp"

msg_len_t chat_message::decode_body_len()
{
    msg_len_t len;
    std::memcpy(&len, &msg_data[0], sizeof(len));
    return ntohl(len);
}

bool chat_message::decode_header()
{
    body_len = decode_body_len();

    if (body_len > chat_message::max_body_len)
    {
        body_len = 0;
        return false;
    }
    return true;
}

void chat_message::encode_header()
{
    msg_len_t len = htonl(body_len);
    std::memcpy(&msg_data[0], &len, sizeof(len));
}

bool chat_message::from_str(uint8_t msg_type, char const *str, size_t len)
{
    if (len > chat_message::max_body_len)
    {
        return false;
    }

    body_len = len+1;
    encode_header();
    body()[0] = msg_type;
    std::memcpy(body() + 1, str, len);

    return true;
}