#pragma once
#include "chat_message.hpp"

struct connection_type
{
    virtual ~connection_type() {}
    
    virtual void send_msg(chat_message const &msg) = 0;
};