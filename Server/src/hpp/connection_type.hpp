#pragma once

struct connection_type
{
    virtual ~connection_type() {}
    
    virtual void send_msg(chat_message const &msg) = 0;

    virtual uint64_t id() const = 0;

    virtual void id(uint64_t new_id) = 0;
};