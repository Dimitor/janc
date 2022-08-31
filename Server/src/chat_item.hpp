#pragma once
#include <string>

struct chat_item
{
    chat_item(){}

    chat_item(uint64_t chat_id, std::string const &chat_name, bool is_single_chat)
    : id{chat_id}
    , name{chat_name}
    , is_single{is_single_chat}
    {}

    uint64_t id;
    std::string name;
    bool is_single;
};