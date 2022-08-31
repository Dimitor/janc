#pragma once

struct chat_item
{
    chat_item(){}

    chat_item(uint64_t chat_id, std::string const &chat_name)
    : id  { chat_id }
    , name{ chat_name }
    {}  

    uint64_t id;
    std::string name;
};