#pragma once
#include <string>
#include <utility>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <fstream>
#include <exception>
#include <vector>
#include <memory>
#include <optional>
#include <ctime>
#include <tuple>
#include <boost/stacktrace.hpp>
#include <mariadb/conncpp.hpp>
#include "hasher.hpp"
#include "message_item.hpp"
#include "chat_item.hpp"

using strcref = std::string const &;
using idtype  = uint64_t;
using idname_t = std::pair<std::vector<idtype>, std::vector<std::string>>;
using chat_data_t = std::tuple<std::vector<idtype>, std::vector<std::string>, std::vector<idtype>>;

struct database
{
    database(std::string &&dbname);

    ~database() { conn->close(); }

    std::optional<idtype> get_user_id(strcref username);

    std::optional<idtype> authorize(strcref username, strcref password);

    std::optional<idtype> add_record(strcref username, strcref password);

    std::optional<message_item> save_message(idtype sender_id, idtype chat_id, strcref msg);

    idtype create_chat(strcref chat_name, std::vector<idtype> const &participants_ids, bool is_single_chat);

    bool delete_chat(idtype chat_id);

    std::vector<std::string> get_usernames(std::vector<idtype> const &ids);

    chat_data_t get_chats(idtype user_id);
    
    idname_t get_chat_participants(idtype chat_id);

    idname_t search_users(strcref search_str, size_t quantity);

    bool change_user_name(idtype user_id, strcref new_username);

    bool change_password(idtype user_id, strcref new_password); 

    std::vector<message_item> get_messages(idtype chat_id, idtype last_msg_id, size_t count);

   // bool get_unread_messages(idtype user_id);

    bool is_user_chat_participant(idtype user_id, idtype chat_id);

private:

    bool add_participants(idtype chat_id, std::vector<idtype> const &participants_ids);

    std::optional<idtype> last_insert_id(strcref table_name);

    static std::pair<std::string, std::string> split_record(strcref record);

    time_t str_to_time_t(strcref strtime);
    
    std::unique_ptr<sql::Connection> conn;
};