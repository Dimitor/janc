#pragma once

using idtype = uint64_t;

struct message_item
{
    message_item(idtype msg_id, std::string const &msg_content, idtype msg_chat_id, idtype msg_user_id, uint64_t msg_creation_time)
    : id{ msg_id }
    , content{ msg_content }
    , chat_id{ msg_chat_id }
    , user_id{ msg_user_id }
    , creation_time{ msg_creation_time }
    {}

    idtype id;
    std::string content;
    idtype chat_id;
    idtype user_id;
    uint64_t creation_time;
};