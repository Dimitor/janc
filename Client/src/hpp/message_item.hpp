#pragma once

using idtype = uint64_t;

struct message_item
{
    message_item(idtype msg_id, std::string const &msg_content, std::string const &msg_sender_name, uint64_t msg_creation_time)
    : id{ msg_id }
    , content{ msg_content }
    , sender_name{ msg_sender_name }
    , creation_time{ msg_creation_time }
    {}

    idtype id;
    std::string content;
    std::string sender_name;
    uint64_t creation_time;
};