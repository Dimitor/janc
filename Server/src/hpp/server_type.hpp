#pragma once
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "database.hpp"
#include "message_parser.hpp"
#include "connection_type.hpp"
#include "chat_message.hpp"
#include "message_item.hpp"
#include "chat_item.hpp"

using connection_ptr = std::shared_ptr<connection_type>;

//class for managing connections and processing messages
struct server_type
{
    server_type(database &db)
    : db{ db }
    {}

    int join(uint64_t connection_id, connection_ptr connection);

    int leave(uint64_t connection_id, connection_ptr connection);

    bool process_message(chat_message const &msg, connection_ptr connection);

    bool process_record(chat_message const &rec_msg, connection_ptr connection);

    void send_msg_to_chat_participants(chat_message const &msg, std::vector<uint64_t> const &chat_participants);

private:

    database &db;
    std::unordered_map<uint64_t, std::vector<connection_ptr>> connections;
};