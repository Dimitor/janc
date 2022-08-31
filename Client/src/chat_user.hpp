#pragma once

struct chat_user
{
    chat_user(){}

    chat_user(uint64_t user_id, std::string &&user_name)
    : id  { user_id }
    , name{ std::move(user_name) }
    {}  

    uint64_t id;
    std::string name;
};